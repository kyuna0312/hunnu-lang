//! LLVM IR code generation for the Hunnu compiler.
//!
//! Converts [`ASTNode`] trees into LLVM IR, object files, or assembly.
//! Only available with the `llvm-codegen` feature.

use llvm_sys::prelude::*;
use llvm_sys::*;
use llvm_sys::core::*;
use super::ast::*;
use super::lexer::TokenType;
use std::ffi::{CStr, CString};
use std::ptr;

/// LLVM IR code generator for Hunnu programs.
pub struct CodeGen {
    context: LLVMContextRef,
    module: LLVMModuleRef,
    builder: LLVMBuilderRef,
    named_values: std::collections::HashMap<String, LLVMValueRef>,
    printf_fn: Option<LLVMValueRef>,
    current_function: Option<LLVMValueRef>,
}

impl CodeGen {
    /// Create a new code generator with the given module name.
    pub fn new(module_name: &str) -> Self {
        unsafe {
            let context = LLVMContextCreate();
            let name = CString::new(module_name).unwrap();
            let module = LLVMModuleCreateWithNameInContext(name.as_ptr(), context);
            let builder = LLVMCreateBuilderInContext(context);

            CodeGen {
                context,
                module,
                builder,
                named_values: std::collections::HashMap::new(),
                printf_fn: None,
                current_function: None,
            }
        }
    }

    /// Generate LLVM IR for a parsed program AST.
    pub fn generate(&mut self, program: &ASTNode) -> Result<(), String> {
        match &program.data {
            NodeData::Program { statements } => {
                for stmt in statements {
                    self.generate_statement(stmt)?;
                }
                Ok(())
            }
            _ => Err("Expected program node".to_string()),
        }
    }

    fn generate_statement(&mut self, node: &ASTNode) -> Result<(), String> {
        match &node.data {
            NodeData::VarDecl { name, initializer } => {
                self.generate_var_decl(name, initializer)
            }
            NodeData::FnDecl { name, params, body } => {
                self.generate_fn_decl(name, params, body)
            }
            NodeData::PrintStmt { argument } => {
                self.generate_print(argument)
            }
            NodeData::Block { statements } => {
                for stmt in statements {
                    self.generate_statement(stmt)?;
                }
                Ok(())
            }
            NodeData::ExprStmt { expression } => {
                self.generate_expression(expression)?;
                Ok(())
            }
            NodeData::ReturnStmt { value } => {
                match value {
                    Some(v) => {
                        let val = self.generate_expression(v)?;
                        unsafe { LLVMBuildRet(self.builder, val); }
                    }
                    None => {
                        unsafe { LLVMBuildRetVoid(self.builder); }
                    }
                }
                Ok(())
            }
            _ => {
                self.generate_expression(node)?;
                Ok(())
            }
        }
    }

    fn generate_var_decl(&mut self, name: &str, initializer: &ASTNode) -> Result<(), String> {
        let value = self.generate_expression(initializer)?;

        unsafe {
            let c_name = CString::new(name).unwrap();
            let i64_type = LLVMInt64TypeInContext(self.context);

            let alloca = LLVMBuildAlloca(self.builder, i64_type, c_name.as_ptr());
            LLVMBuildStore(self.builder, value, alloca);

            self.named_values.insert(name.to_string(), alloca);
        }

        Ok(())
    }

    fn generate_fn_decl(&mut self, name: &str, params: &[String], body: &ASTNode) -> Result<(), String> {
        unsafe {
            let c_name = CString::new(name).unwrap();
            let i64_type = LLVMInt64TypeInContext(self.context);
            let void_type = LLVMVoidTypeInContext(self.context);

            let is_main = name == "main";
            let return_type = if is_main { i64_type } else { void_type };

            let param_count = params.len();
            let mut param_types: Vec<LLVMTypeRef> = Vec::new();
            for _ in params {
                param_types.push(i64_type);
            }

            let function_type = LLVMFunctionType(
                return_type,
                param_types.as_mut_ptr(),
                param_count as u32,
                0,
            );

            let function = LLVMAddFunction(self.module, c_name.as_ptr(), function_type);

            if is_main {
                let target_triple = LLVMGetDefaultTargetTriple();
                LLVMSetTarget(self.module, target_triple);
            }

            let entry = LLVMAppendBasicBlockInContext(self.context, function, c"entry".as_ptr());
            LLVMPositionBuilderAtEnd(self.builder, entry);

            self.current_function = Some(function);

            for (i, param_name) in params.iter().enumerate() {
                let c_param_name = CString::new(param_name.as_str()).unwrap();
                let param = LLVMGetParam(function, i as u32);
                LLVMSetValueName(param, c_param_name.as_ptr());

                let alloca = LLVMBuildAlloca(self.builder, i64_type, c_param_name.as_ptr());
                LLVMBuildStore(self.builder, param, alloca);
                self.named_values.insert(param_name.to_string(), alloca);
            }

            self.generate_statement(body)?;

            let last_instruction = LLVMGetLastInstruction(LLVMGetBasicBlockTerminator(entry));
            if last_instruction.is_null() {
                if is_main {
                    let zero = LLVMConstInt(i64_type, 0, 0);
                    LLVMBuildRet(self.builder, zero);
                } else {
                    LLVMBuildRetVoid(self.builder);
                }
            }

            self.named_values.clear();

            Ok(())
        }
    }

    fn generate_print(&mut self, argument: &ASTNode) -> Result<(), String> {
        unsafe {
            if self.printf_fn.is_none() {
                let i32_type = LLVMInt32TypeInContext(self.context);
                let i8_type = LLVMInt8TypeInContext(self.context);
                let i8_ptr = LLVMPointerType(i8_type, 0);

                let mut printf_param_types = vec![i8_ptr];
                let printf_type = LLVMFunctionType(
                    i32_type,
                    printf_param_types.as_mut_ptr(),
                    1,
                    1,
                );
                let printf_name = CString::new("printf").unwrap();
                let printf = LLVMAddFunction(self.module, printf_name.as_ptr(), printf_type);
                self.printf_fn = Some(printf);
            }
            let printf = self.printf_fn.unwrap();

            match &argument.data {
                NodeData::Literal { literal_type, value } => {
                    match literal_type {
                        LiteralType::Int => {
                            let fmt = CString::new("%lld\n\0").unwrap();
                            let global = LLVMAddGlobal(self.module, LLVMArrayType(LLVMInt8TypeInContext(self.context), fmt.as_bytes().len() as u32), c"int_fmt".as_ptr());
                            LLVMSetInitializer(global, LLVMConstStringInContext(self.context, fmt.as_ptr(), fmt.as_bytes().len() as u32, 1));
                            LLVMSetLinkage(global, LLVMLinkage::LLVMPrivateLinkage);
                            LLVMSetGlobalConstant(global, 1);

                            let zero = LLVMConstInt(LLVMInt32TypeInContext(self.context), 0, 0);
                            let gep_indices = [zero, zero];
                            let fmt_ptr = LLVMBuildInBoundsGEP2(self.builder,
                                LLVMArrayType(LLVMInt8TypeInContext(self.context), fmt.as_bytes().len() as u32),
                                global,
                                gep_indices.as_mut_ptr(),
                                2,
                                c"fmt".as_ptr());

                            let value = self.generate_expression(argument)?;
                            let mut args = [fmt_ptr, value];
                            LLVMBuildCall2(self.builder,
                                LLVMGlobalGetValueType(printf),
                                printf,
                                args.as_mut_ptr(),
                                2,
                                c"printf_call".as_ptr());
                        }
                        LiteralType::String => {
                            let s = match value {
                                LiteralValue::String(s) => s.clone(),
                                _ => String::new(),
                            };
                            let c_string = CString::new(s.as_str()).unwrap_or(CString::new("").unwrap());
                            let null_term = format!("{}\0", c_string.to_str().unwrap_or(""));
                            let global = LLVMAddGlobal(self.module, LLVMArrayType(LLVMInt8TypeInContext(self.context), null_term.len() as u32), c"str".as_ptr());
                            LLVMSetInitializer(global, LLVMConstStringInContext(self.context, null_term.as_ptr(), null_term.len() as u32, 1));
                            LLVMSetLinkage(global, LLVMLinkage::LLVMPrivateLinkage);
                            LLVMSetGlobalConstant(global, 1);

                            let zero = LLVMConstInt(LLVMInt32TypeInContext(self.context), 0, 0);
                            let gep_indices = [zero, zero];
                            let str_ptr = LLVMBuildInBoundsGEP2(self.builder,
                                LLVMArrayType(LLVMInt8TypeInContext(self.context), null_term.len() as u32),
                                global,
                                gep_indices.as_mut_ptr(),
                                2,
                                c"str_ptr".as_ptr());

                            let mut args = [str_ptr];
                            LLVMBuildCall2(self.builder,
                                LLVMGlobalGetValueType(printf),
                                printf,
                                args.as_mut_ptr(),
                                1,
                                c"printf_call".as_ptr());
                        }
                        _ => {
                            let fmt = CString::new("%lld\n\0").unwrap();
                            let global = LLVMAddGlobal(self.module, LLVMArrayType(LLVMInt8TypeInContext(self.context), fmt.as_bytes().len() as u32), c"fmt_str".as_ptr());
                            LLVMSetInitializer(global, LLVMConstStringInContext(self.context, fmt.as_ptr(), fmt.as_bytes().len() as u32, 1));
                            LLVMSetLinkage(global, LLVMLinkage::LLVMPrivateLinkage);
                            LLVMSetGlobalConstant(global, 1);

                            let zero = LLVMConstInt(LLVMInt32TypeInContext(self.context), 0, 0);
                            let gep_indices = [zero, zero];
                            let fmt_ptr = LLVMBuildInBoundsGEP2(self.builder,
                                LLVMArrayType(LLVMInt8TypeInContext(self.context), fmt.as_bytes().len() as u32),
                                global,
                                gep_indices.as_mut_ptr(),
                                2,
                                c"fmt".as_ptr());

                            let val = LLVMConstInt(LLVMInt64TypeInContext(self.context), 0, 0);
                            let mut args = [fmt_ptr, val];
                            LLVMBuildCall2(self.builder,
                                LLVMGlobalGetValueType(printf),
                                printf,
                                args.as_mut_ptr(),
                                2,
                                c"printf_call".as_ptr());
                        }
                    }
                }
                _ => {
                    let value = self.generate_expression(argument)?;
                    let fmt = CString::new("%lld\n\0").unwrap();
                    let global = LLVMAddGlobal(self.module, LLVMArrayType(LLVMInt8TypeInContext(self.context), fmt.as_bytes().len() as u32), c"fmt_expr".as_ptr());
                    LLVMSetInitializer(global, LLVMConstStringInContext(self.context, fmt.as_ptr(), fmt.as_bytes().len() as u32, 1));
                    LLVMSetLinkage(global, LLVMLinkage::LLVMPrivateLinkage);
                    LLVMSetGlobalConstant(global, 1);

                    let zero = LLVMConstInt(LLVMInt32TypeInContext(self.context), 0, 0);
                    let gep_indices = [zero, zero];
                    let fmt_ptr = LLVMBuildInBoundsGEP2(self.builder,
                        LLVMArrayType(LLVMInt8TypeInContext(self.context), fmt.as_bytes().len() as u32),
                        global,
                        gep_indices.as_mut_ptr(),
                        2,
                        c"fmt".as_ptr());

                    let mut args = [fmt_ptr, value];
                    LLVMBuildCall2(self.builder,
                        LLVMGlobalGetValueType(printf),
                        printf,
                        args.as_mut_ptr(),
                        2,
                        c"printf_call".as_ptr());
                }
            }
        }

        Ok(())
    }

    fn generate_expression(&mut self, node: &ASTNode) -> Result<LLVMValueRef, String> {
        match &node.data {
            NodeData::Literal { literal_type, value } => {
                self.generate_literal(literal_type, value)
            }
            NodeData::Identifier { name } => {
                self.generate_identifier(name)
            }
            NodeData::BinaryExpr { operator, left, right } => {
                self.generate_binary_expr(operator, left, right)
            }
            NodeData::CallExpr { name, args } => {
                self.generate_call_expr(name, args)
            }
            _ => {
                unsafe {
                    Ok(LLVMConstInt(LLVMInt64TypeInContext(self.context), 0, 0))
                }
            }
        }
    }

    fn generate_literal(&mut self, literal_type: &LiteralType, value: &LiteralValue) -> Result<LLVMValueRef, String> {
        unsafe {
            match literal_type {
                LiteralType::Int => {
                    if let LiteralValue::Int(v) = value {
                        Ok(LLVMConstInt(LLVMInt64TypeInContext(self.context), *v as u64, 0))
                    } else {
                        Err("Expected int literal".to_string())
                    }
                }
                LiteralType::Float => {
                    if let LiteralValue::Float(v) = value {
                        Ok(LLVMConstReal(LLVMDoubleTypeInContext(self.context), *v))
                    } else {
                        Err("Expected float literal".to_string())
                    }
                }
                LiteralType::Bool => {
                    if let LiteralValue::Bool(v) = value {
                        Ok(LLVMConstInt(LLVMInt1TypeInContext(self.context), if *v { 1 } else { 0 }, 0))
                    } else {
                        Err("Expected bool literal".to_string())
                    }
                }
                LiteralType::String => {
                    if let LiteralValue::String(s) = value {
                        let c_string = CString::new(s.as_str()).unwrap_or(CString::new("").unwrap());
                        let null_term = format!("{}\0", c_string.to_str().unwrap_or(""));
                        let name = CString::new(format!("str_{}", s.len())).unwrap();
                        let global = LLVMAddGlobal(self.module,
                            LLVMArrayType(LLVMInt8TypeInContext(self.context), null_term.len() as u32),
                            name.as_ptr());
                        LLVMSetInitializer(global, LLVMConstStringInContext(self.context,
                            null_term.as_ptr(), null_term.len() as u32, 1));
                        LLVMSetLinkage(global, LLVMLinkage::LLVMPrivateLinkage);
                        LLVMSetGlobalConstant(global, 1);

                        let zero = LLVMConstInt(LLVMInt32TypeInContext(self.context), 0, 0);
                        let gep_indices = [zero, zero];
                        let ptr = LLVMBuildInBoundsGEP2(self.builder,
                            LLVMArrayType(LLVMInt8TypeInContext(self.context), null_term.len() as u32),
                            global,
                            gep_indices.as_mut_ptr(),
                            2,
                            c"str".as_ptr());
                        Ok(ptr)
                    } else {
                        Err("Expected string literal".to_string())
                    }
                }
            }
        }
    }

    fn generate_identifier(&mut self, name: &str) -> Result<LLVMValueRef, String> {
        unsafe {
            if let Some(alloca) = self.named_values.get(name) {
                let i64_type = LLVMInt64TypeInContext(self.context);
                let loaded = LLVMBuildLoad2(self.builder, i64_type, *alloca, c"load".as_ptr());
                Ok(loaded)
            } else {
                let c_name = CString::new(name).unwrap();
                let global = LLVMGetNamedGlobal(self.module, c_name.as_ptr());
                if !global.is_null() {
                    let i64_type = LLVMInt64TypeInContext(self.context);
                    Ok(LLVMBuildLoad2(self.builder, i64_type, global, c"load".as_ptr()))
                } else {
                    Ok(LLVMConstInt(LLVMInt64TypeInContext(self.context), 0, 0))
                }
            }
        }
    }

    fn generate_binary_expr(&mut self, operator: &TokenType, left: &ASTNode, right: &ASTNode) -> Result<LLVMValueRef, String> {
        let lhs = self.generate_expression(left)?;
        let rhs = self.generate_expression(right)?;

        unsafe {
            match operator {
                TokenType::Plus => {
                    Ok(LLVMBuildAdd(self.builder, lhs, rhs, c"add".as_ptr()))
                }
                TokenType::Minus => {
                    Ok(LLVMBuildSub(self.builder, lhs, rhs, c"sub".as_ptr()))
                }
                TokenType::Star => {
                    Ok(LLVMBuildMul(self.builder, lhs, rhs, c"mul".as_ptr()))
                }
                TokenType::Slash => {
                    Ok(LLVMBuildSDiv(self.builder, lhs, rhs, c"div".as_ptr()))
                }
                TokenType::Percent => {
                    Ok(LLVMBuildSRem(self.builder, lhs, rhs, c"rem".as_ptr()))
                }
                TokenType::Eq => {
                    let cmp = LLVMBuildICmp(self.builder, LLVMIntPredicate::LLVMIntEQ, lhs, rhs, c"eq".as_ptr());
                    Ok(LLVMBuildIntCast2(self.builder, cmp, LLVMInt64TypeInContext(self.context), 0, c"eq_int".as_ptr()))
                }
                TokenType::Neq => {
                    let cmp = LLVMBuildICmp(self.builder, LLVMIntPredicate::LLVMIntNE, lhs, rhs, c"ne".as_ptr());
                    Ok(LLVMBuildIntCast2(self.builder, cmp, LLVMInt64TypeInContext(self.context), 0, c"ne_int".as_ptr()))
                }
                TokenType::Lt => {
                    let cmp = LLVMBuildICmp(self.builder, LLVMIntPredicate::LLVMIntSLT, lhs, rhs, c"lt".as_ptr());
                    Ok(LLVMBuildIntCast2(self.builder, cmp, LLVMInt64TypeInContext(self.context), 0, c"lt_int".as_ptr()))
                }
                TokenType::Le => {
                    let cmp = LLVMBuildICmp(self.builder, LLVMIntPredicate::LLVMIntSLE, lhs, rhs, c"le".as_ptr());
                    Ok(LLVMBuildIntCast2(self.builder, cmp, LLVMInt64TypeInContext(self.context), 0, c"le_int".as_ptr()))
                }
                TokenType::Gt => {
                    let cmp = LLVMBuildICmp(self.builder, LLVMIntPredicate::LLVMIntSGT, lhs, rhs, c"gt".as_ptr());
                    Ok(LLVMBuildIntCast2(self.builder, cmp, LLVMInt64TypeInContext(self.context), 0, c"gt_int".as_ptr()))
                }
                TokenType::Ge => {
                    let cmp = LLVMBuildICmp(self.builder, LLVMIntPredicate::LLVMIntSGE, lhs, rhs, c"ge".as_ptr());
                    Ok(LLVMBuildIntCast2(self.builder, cmp, LLVMInt64TypeInContext(self.context), 0, c"ge_int".as_ptr()))
                }
                _ => Ok(lhs),
            }
        }
    }

    fn generate_call_expr(&mut self, name: &str, args: &[ASTNode]) -> Result<LLVMValueRef, String> {
        unsafe {
            let c_name = CString::new(name).unwrap();
            let callee = LLVMGetNamedFunction(self.module, c_name.as_ptr());
            if callee.is_null() {
                return Err(format!("Unknown function '{}'", name));
            }

            let mut arg_values = Vec::new();
            for arg in args {
                let val = self.generate_expression(arg)?;
                arg_values.push(val);
            }

            let result = LLVMBuildCall2(
                self.builder,
                LLVMGlobalGetValueType(callee),
                callee,
                arg_values.as_mut_ptr(),
                arg_values.len() as u32,
                c"call".as_ptr(),
            );
            Ok(result)
        }
    }

    /// Print the generated LLVM IR to stderr.
    pub fn print_ir(&self) {
        unsafe {
            LLVMDumpModule(self.module);
        }
    }

    /// Write LLVM bitcode to a file.
    pub fn write_bitcode(&self, filename: &str) -> Result<(), String> {
        unsafe {
            let c_filename = CString::new(filename).unwrap();
            let result = LLVMWriteBitcodeToFile(self.module, c_filename.as_ptr());
            if result != 0 {
                Err(format!("Failed to write bitcode to '{}'", filename))
            } else {
                Ok(())
            }
        }
    }

    /// Emit a native object file (.o).
    pub fn emit_object(&self, filename: &str) -> Result<(), String> {
        unsafe {
            let triple = LLVMGetDefaultTargetTriple();
            LLVMSetTarget(self.module, triple);

            let mut target: LLVMTargetRef = ptr::null_mut();
            let mut error: *mut c_char = ptr::null_mut();
            if LLVMGetTargetFromTriple(triple, &mut target, &mut error) != 0 {
                let err_str = CStr::from_ptr(error).to_string_lossy().into_owned();
                LLVMDisposeMessage(error);
                return Err(format!("Failed to get target: {}", err_str));
            }

            let machine = LLVMCreateTargetMachine(
                target,
                triple,
                c"generic".as_ptr(),
                c"".as_ptr(),
                LLVMCodeGenOptLevel::LLVMCodeGenLevelDefault,
                LLVMRelocMode::LLVMRelocDefault,
                LLVMCodeModel::LLVMCodeModelDefault,
            );

            let c_filename = CString::new(filename).unwrap();
            LLVMTargetMachineEmitToFile(
                machine,
                self.module,
                c_filename.as_ptr() as *mut c_char,
                LLVMCodeGenFileType::LLVMObjectFile,
                &mut error,
            );

            if !error.is_null() {
                let err_str = CStr::from_ptr(error).to_string_lossy().into_owned();
                LLVMDisposeMessage(error);
                LLVMDisposeTargetMachine(machine);
                return Err(format!("Failed to emit object: {}", err_str));
            }

            LLVMDisposeTargetMachine(machine);
            Ok(())
        }
    }

    /// Emit assembly text (.s).
    pub fn emit_assembly(&self, filename: &str) -> Result<(), String> {
        unsafe {
            let triple = LLVMGetDefaultTargetTriple();
            LLVMSetTarget(self.module, triple);

            let mut target: LLVMTargetRef = ptr::null_mut();
            let mut error: *mut c_char = ptr::null_mut();
            if LLVMGetTargetFromTriple(triple, &mut target, &mut error) != 0 {
                let err_str = CStr::from_ptr(error).to_string_lossy().into_owned();
                LLVMDisposeMessage(error);
                return Err(format!("Failed to get target: {}", err_str));
            }

            let machine = LLVMTargetMachineCreate(
                target,
                triple,
                c"generic".as_ptr(),
                c"".as_ptr(),
                LLVMCodeGenOptLevel::LLVMCodeGenLevelDefault,
                LLVMRelocMode::LLVMRelocPIC,
                LLVMCodeModel::LLVMCodeModelDefault,
            );

            let c_filename = CString::new(filename).unwrap();
            LLVMTargetMachineEmitToFile(
                machine,
                self.module,
                c_filename.as_ptr() as *mut c_char,
                LLVMCodeGenFileType::LLVMAssemblyFile,
                &mut error,
            );

            if !error.is_null() {
                let err_str = CStr::from_ptr(error).to_string_lossy().into_owned();
                LLVMDisposeMessage(error);
                LLVMDisposeTargetMachine(machine);
                return Err(format!("Failed to emit assembly: {}", err_str));
            }

            LLVMDisposeTargetMachine(machine);
            Ok(())
        }
    }
}

impl Drop for CodeGen {
    fn drop(&mut self) {
        unsafe {
            LLVMDisposeBuilder(self.builder);
            LLVMDisposeModule(self.module);
            LLVMContextDispose(self.context);
        }
    }
}

/// Convenience: compile an AST to LLVM IR and write to a file (or print if None).
pub fn compile_to_ir(program: &ASTNode, output_file: Option<&str>) -> Result<(), String> {
    let mut codegen = CodeGen::new("hunnu_module");
    codegen.generate(program)?;

    if let Some(filename) = output_file {
        codegen.write_bitcode(filename)?
    } else {
        codegen.print_ir();
    }

    Ok(())
}

/// Convenience: compile an AST to a native object file.
pub fn compile_to_object(program: &ASTNode, output_file: &str) -> Result<(), String> {
    let mut codegen = CodeGen::new("hunnu_module");
    codegen.generate(program)?;
    codegen.emit_object(output_file)
}

/// Convenience: compile an AST to assembly text.
pub fn compile_to_assembly(program: &ASTNode, output_file: &str) -> Result<(), String> {
    let mut codegen = CodeGen::new("hunnu_module");
    codegen.generate(program)?;
    codegen.emit_assembly(output_file)
}
