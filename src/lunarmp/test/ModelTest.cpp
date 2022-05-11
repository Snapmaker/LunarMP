//
// Created by Cyril on 2022/4/28.
//

#include "ModelTest.h"

namespace lunarmp {

void splitPath(const std::string& str, const std::string& sp, std::vector<std::string>& vec) {
    size_t size = sp.size();

    vec.clear();
    size_t end = 0, start = 0;
    while (start != std::string::npos && start < str.size()) {
        end = str.find(sp, start);
        vec.push_back(str.substr(start, end - start));  // end == 0 时压入空字符串
        start = end == std::string::npos ? end : end + size;
    }

    if (vec.empty()) vec.push_back(str);
}

void ModelTest::GetSpecialFilesFromDirectory(std::string path, std::string fileType, std::vector<std::string>& files) {
//    std::vector<std::string> tempFileTypes;
//    splitPath(fileType, " ", tempFileTypes);
//    if (tempFileTypes.size() == 0) return;
//
//    for (int i = 0; i < tempFileTypes.size(); ++i) {
//        // 文件句柄
//        __int64 hFile = 0;
//        // 文件信息
//        struct _finddata_t fileinfo;
//
//        std::string p;
//
//        if ((hFile = _findfirst(p.assign(path).append("\\*" + tempFileTypes[i]).c_str(), &fileinfo)) != -1) {
//            do {
//                // 第一种：保存文件的全路径
//                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
//                //第二种：不保存文件的全路径
//                // files.push_back(fileinfo.name);
//            } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
//
//            _findclose(hFile);
//        }
//    }
}

void ModelTest::GetAllFilesIncludeSubfolder(std::string path, std::string fileType, std::vector<std::string>& files) {
//    std::vector<std::string> tempFileTypes;
//    splitPath(fileType, "|", tempFileTypes);
//    if (tempFileTypes.size() == 0) return;
//    //文件句柄
//    __int64 hFile = 0;
//    //文件信息
//    struct _finddata_t fileinfo;
//    std::string p;
//    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
//        do {
//            if ((fileinfo.attrib & _A_SUBDIR)) {  //比较文件类型是否是文件夹
//                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
//                    std::string subForld = p.assign(path).append("\\").append(fileinfo.name);
//
//                    //递归搜索子文件夹
//                    GetAllFilesIncludeSubfolder(p.assign(path).append("\\").append(fileinfo.name), fileType, files);
//                }
//            } else {
//                for (int i = 0; i < tempFileTypes.size(); ++i) {
//                    std::string fileName = fileinfo.name;
//                    if (fileName.find(tempFileTypes[i]) != std::string::npos) {
//                        // 第一种：保存文件的全路径
//                        files.push_back(p.assign(path).append("\\").append(fileinfo.name));
//
//                        //第二种：不保存文件的全路径
//                        // files.push_back(fileinfo.name);
//                    }
//                }
//            }
//        } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
//        _findclose(hFile);
//    }
}

int ModelTest::getFileSize(std::string filepath) {
    std::ifstream fin(filepath);
    int size;
    if (fin.is_open()) {
        fin.seekg(0, std::ios::end);
        size = fin.tellg();
        fin.close();
        std::cout << size << std::endl;
    }
    return size;
}

std::string getFileName(std::string str, std::string pattern) {
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    int size = str.size();
    for (int i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if (pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result[result.size() - 1];
}
int ModelTest::simpfyOutput(double mSize) {
    std::vector<std::string> m_Files;
    std::string path = "E:\\Datasets\\3d\\123456789101112\\";
    GetAllFilesIncludeSubfolder(path, ".stl", m_Files);
    std::ofstream outFile;
    outFile.open("E:\\C\\12.csv", std::ios::out);
    outFile << "File" << ',' << "Size" << ',' << "simplification_time" << ',' << "Edges_0" << ',' << "Faces_0" << ',' << "Points_0" << ',' << "Edges_1" << ','
            << "Faces_1" << ',' << "Points_1" << ',' << "compare_time" << ',' << "hausdorff_bounded_error_distance" << ',' << std::endl;

    for (int i = 0; i < m_Files.size(); ++i) {
        int size = getFileSize((m_Files[i]));

        std::cout << m_Files[i] << "\tsize: " << size << std::endl;

        ModelSimplification ms;
        ModelCompare mcp;
        std::string patten = "\\";
        std::string outFiles = "E:\\C\\" + getFileName(m_Files[i], patten);

        Mesh mesh, mesh2;

        //ms.readFile(m_Files[i], mesh);
        //        ms.initSetting(mSize);
        ms.modelSimplification(m_Files[i], outFiles, 1, 300, 0.08);
        /*if (ms.remove_edge > 0) {
            mcp.modelCompare(mesh, mesh2, 0.01);
        }*/
        outFile << m_Files[i] << ',' << size << ',' << ms.simplification_time << ',' << mesh.number_of_halfedges() / 2 << ',' << mesh.number_of_faces() << ','
                << mesh.number_of_vertices() << ',' << mesh2.number_of_halfedges() / 2 << ',' << mesh2.number_of_faces() << ',' << mesh2.number_of_vertices()
                << ',' << mcp.compare_time << ',' << mcp.hausdorff_bounded_error_distance << ',' << std::endl;
    }
    outFile.close();
    return 0;
}

int ModelTest::simpfyOutput() {
    simpfyOutput(0.75);
    //    simpfyOutput(0.25);
    //    simpfyOutput(0.1);
    //    simpfyOutput(0.05);
    return 0;
}

int ModelTest::testOutput() {
//    std::vector<std::string> m_Files;
//    std::string path = "E:\\Datasets\\3d\\123456789101112\\";
//    GetAllFilesIncludeSubfolder(path, ".stl", m_Files);
//
//    std::ofstream outFile;
//    outFile.open("E:\\Datasets\\repaired\\12.csv", std::ios::out);
//    outFile << "File" << ',' << "Size" << ',' << "Defective" << ',' << "Check_Time" << ',' << "non_manifold_edge" << ',' << "non_manifold_vertex" << ','
//            << "duplicated_vertex" << ',' << "polygon_orientation_reversed" << ',' << "number_of_connected_components" << ',' << "number_of_holes" << ','
//            << "number_of_intersections" << ',' << "Repair_Time" << ',' << "repair_basic_time" << ',' << "read_file_time" << ',' << "repair_manifoldness_time"
//            << ',' << "repair_borders_time" << ',' << "repair_holes_time" << ',' << "repair_self_intersect_time" << std::endl;
//    //            << "repair_borders_time" << ',' << "repair_holes_time" << ',' << "repair_self_intersect_time" << ','
//    //            << "simplification_time" << ',' << "compare_time" << ',' << "hausdorff_bounded_error_distance" << std::endl;
//
//    for (int i = 0; i < m_Files.size(); ++i) {
//        int size = getFileSize((m_Files[i]));
//
//        std::cout << m_Files[i] << "\tsize: " << size << std::endl;
//
//        //        if (size == 150000000) {
//        //            continue;
//        //        }
//        Mesh mesh;
//        ModelCheck mc;
//        ModelRepair mr;
//        ModelSimplification ms;
//        ModelCompare mcp;
//
//        //bool f = false;
//        bool f = mc.checkTest(m_Files[i]);
//        //        if (f) {
//        //            std::string patten = "\\";
//        //            std::string outFiles = "E:\\A\\3D_model1\\" + getFileName(m_Files[i], patten);
//        //            std::cout << outFiles << std::endl;
//        //            mesh = mr.repairModelMesh(m_Files[i], outFiles);
//        ////            mesh = ms.modelSimplification(mesh, 0.2);
//        //        }
//        //        else {
//        ////            mesh = ms.modelSimplification(m_Files[i], 0.2);
//        //        }
//        std::string patten = "\\";
//        std::string outFiles = "E:\\Datasets\\repaired\\12\\" + getFileName(m_Files[i], patten);
//        std::cout << outFiles << std::endl;
//        mr.repairModel(m_Files[i], outFiles);
//        //        mcp.modelCompare(m_Files[i], mesh, 0.01);
//        outFile << outFiles << ',' << size << ',' << f << ',' << mc.check_time << ',' << mr.non_manifold_edge << ',' << mr.non_manifold_vertex << ','
//                << mr.duplicated_vertex << ',' << mr.polygon_orientation_reversed << ',' << mr.number_of_connected_components << ',' << mr.number_of_holes
//                << ',' << mr.number_of_intersections << ',' << mr.repair_time << ',' << mr.read_file_time << ',' << mr.repair_basic_time << ','
//                << mr.repair_manifoldness_time << ',' << mr.repair_borders_time << ',' << mr.repair_holes_time << ',' << mr.repair_self_intersect_time
//                << std::endl;
//        //                << mr.repair_borders_time << ',' << mr.repair_holes_time << ',' << mr.repair_self_intersect_time << ','
//        //                << ms.simplification_time << ',' << mcp.compare_time << ',' << mcp.hausdorff_bounded_error_distance << std::endl;
//
//        std::cout << "check: " << mc.check_time << "\trepair: " << mr.repair_time << std::endl;
//        //        mcp.modelCompare(m_Files[i], mesh, 0.01);
//    }
//    outFile.close();
//    return 0;
}
}  // namespace lunarmp
