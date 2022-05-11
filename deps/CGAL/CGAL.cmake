add_deps_cmake_project(
        CGAL
        # GIT_REPOSITORY https://github.com/CGAL/cgal.git
        # GIT_TAG        bec70a6d52d8aacb0b3d82a7b4edc3caa899184b # releases/CGAL-5.0
        # For whatever reason, this keeps downloading forever (repeats downloads if finished)
        URL https://github.com/CGAL/cgal/releases/download/v5.4/CGAL-5.4.zip
        URL_HASH SHA256=b0a16738fac6de5e8d7ead7687c0ad2a72a9307c363d502b7f0e4ae8e2ddeddb
        DEPENDS
)

