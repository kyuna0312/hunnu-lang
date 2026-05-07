use pyo3::prelude::*;
use pyo3::types::PyDict;

/// Python module for Hunnu language
#[pymodule]
fn hunnu(_py: Python, m: &PyModule) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(run_file, m)?)?;
    m.add_function(wrap_pyfunction!(run_code, m)?)?;
    Ok(())
}

/// Run a Hunnu source file
#[pyfunction]
fn run_file(path: &str) -> PyResult<String> {
    // TODO: Call into Hunnu compiler/VM
    Ok(format!("Running Hunnu file: {}", path))
}

/// Run Hunnu code string
#[pyfunction]
fn run_code(code: &str) -> PyResult<String> {
    // TODO: Call into Hunnu compiler/VM
    Ok(format!("Running Hunnu code: {}...", &code[..code.len().min(20)]))
}
