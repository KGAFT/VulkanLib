pub mod vl_semaphore;

use std::ffi::{c_char, CString};

pub fn c_string_vec_to_ptr(strings: &Vec<CString>) -> Vec<*const c_char> {
    strings.iter().map(|e| e.as_ptr()).collect()
}