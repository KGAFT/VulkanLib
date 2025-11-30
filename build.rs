use std::{env, fs, path::PathBuf};

fn main() {
    // Directory you want to copy
    let source = PathBuf::from("glsl");

    // Cargo sets this env variable to the output directory,
    // e.g. target/debug/ or target/release/
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    // OUT_DIR is a subfolder, we want the parent (debug/release)
    let target_dir = out_dir
        .ancestors()
        .nth(3)      // Go up 3 levels: build/<hash>/out → <hash> → build → debug|release
        .unwrap()
        .to_path_buf();

    let dest = target_dir.join("glsl");

    if dest.exists() {
        fs::remove_dir_all(&dest).unwrap();
    }

    fs::create_dir_all(&dest).unwrap();
    copy_dir_recursive(&source, &dest);

    println!("cargo:rerun-if-changed=glsl");
}

fn copy_dir_recursive(src: &PathBuf, dst: &PathBuf) {
    for entry in fs::read_dir(src).unwrap() {
        let entry = entry.unwrap();
        let path = entry.path();
        let dest_path = dst.join(entry.file_name());

        if path.is_dir() {
            fs::create_dir_all(&dest_path).unwrap();
            copy_dir_recursive(&path, &dest_path);
        } else {
            fs::copy(&path, &dest_path).unwrap();
        }
    }
}
