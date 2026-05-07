/**
 * @file codegen.rs
 * @brief LLVM IR code generation for Hunnu Language
 * 
 * This module converts the AST into LLVM IR
 * to produce native binaries (Month 3 feature).
 * 
 * Note: This is a skeleton implementation.
 * Full LLVM integration requires the llvm-sys crate
 * and proper LLVM initialization.
 */

use llvm_sys::*;
use super::ast::*;
use super::lexer::TokenType;
use std::ptr;

pub struct CodeGen {
    context: *mut LLVMContext,
    module: *mut LLVMModule,
    builder: *mut LLVMBuilder,
}

impl CodeGen {
    pub fn new(module_name: &str) -> Self {
        unsafe {
            let context = LLVMContextCreate();
            let module = LLVMModuleCreateWithNameInContext(
                std::ffi::CString::new(module_name).unwrap().as_ptr(),
                context,
            );
            let builder = LLVMCreateBuilderInContext(context);
            
            CodeGen {
                context,
                module,
                builder,
            }
        }
    }
    
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
            _ => {
                let _ = self.generate_expression(node)?;
                Ok(())
            }
        }
    }
    
    fn generate_var_decl(&mut self, name: &str, initializer: &ASTNode) -> Result<(), String> {
        let value = self.generate_expression(initializer)?;
        
        unsafe {
            let c_name = std::ffi::CString::new(name).unwrap();
            let alloca = LLVMSetInitializer(
                LLVMAddGlobal(self.module, LLVMInt64TypeInContext(self.context), c_name.as_ptr()),
                value,
            );
            LLVMSetLinkage(allocea, LLVMLinkage::LLVMLinkOnceAnyLinkage);
        }
        
        Ok(())
    }
    
    fn generate_fn_decl(&mut self, name: &str, params: &[String], body: &ASTNode) -> Result<(), String> {
        unsafe {
            let c_name = std::ffi::CString::new(name).unwrap();
            
            let function_type = LLVMFunctionType(
                LLVMInt64TypeInContext(self.context),
                ptr::null_mut(),
                0,
                0,
            );
            
            let function = LLVMAddFunction(self.module, c_name.as_ptr(), function_type);
            
            let entry = LLVMAppendBasicBlockInContext(self.context, function, c"entry".as_ptr());
            LLVMPositionBuilderAtEnd(self.builder, entry);
            
            self.generate_statement(body)?;
            
            Ok(())
        }
    }
    
    fn generate_print(&mut self, argument: &ASTNode) -> Result<(), String> {
        let value = self.generate_expression(argument)?;
        
        unsafe {
            let printf = LLVMGetNamedFunction(self.module, c"printf".as_ptr());
            
            if printf.is_null() {
                let printf_type = LLVMFunctionType(
                    LLVMInt32TypeInContext(self.context),
                    ptr::null_mut(),
                    0,
                    1,
                );
                LLVMAddFunction(self.module, c"printf".as_ptr(), printf_type);
            }
            
            LLVMSetTailCall(LLVMSetValueName(value, c"arg".as_ptr()), 0);
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
                _ => Ok(LLVMConstInt(LLVMInt64TypeInContext(self.context), 0, 0)),
            }
        }
    }
    
    fn generate_identifier(&mut self, name: &str) -> Result<LLVMValueRef, String> {
        unsafe {
            let c_name = std::ffi::CString::new(name).unwrap();
            let global = LLVMGetNamedGlobal(self.module, c_name.as_ptr());
            if !global.is_null() {
                Ok(LLVMBuildLoad2(self.builder, LLVMInt64TypeInContext(self.context), global, c"load".as_ptr()))
            } else {
                Ok(LLVMConstInt(LLVMInt64TypeInContext(self.context), 0, 0))
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
                _ => Ok(lhs),
            }
        }
    }
    
    pub fn print_ir(&self) {
        unsafe {
            LLVMDumpModule(self.module);
        }
    }
    
    pub fn write_bitcode(&self, filename: &str) {
        unsafe {
            let c_filename = std::ffi::CString::new(filename).unwrap();
            LLVMWriteBitcodeToFile(self.module, c_filename.as_ptr());
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

pub fn compile_to_ir(program: &ASTNode, output_file: Option<&str>) -> Result<(), String> {
    let mut codegen = CodeGen::new("hunnu_module");
    codegen.generate(program)?;
    
    if let Some(filename) = output_file {
        codegen.write_bitcode(filename);
    } else {
        codegen.print_ir();
    }
    
    Ok(())
}
