    include <tread>
    include <sstream>
    
        
double DET(osg::Matrix3 &mat) {
    double r00 = mat(0, 0);
    double r01 = mat(0, 1);
    double r02 = mat(0, 2);
    double r10 = mat(1, 0);
    double r11 = mat(1, 1);
    double r12 = mat(1, 2);
    double r20 = mat(2, 0);
    double r21 = mat(2, 1);
    double r22 = mat(2, 2);

    // Partially compute inverse of rot
    double m00 = r11 * r22 - r12 * r21;
    double m01 = r02 * r21 - r01 * r22;
    double m02 = r01 * r12 - r02 * r11;

    // Compute determinant of rot from 3 elements just computed
    return r00 * m00 + r10 * m01 + r20 * m02;
}

        
    static int count = 0;
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    unsigned long long tid = std::stoull(stid);
    printf("EvnetHandler  frame() %05d\n", tid);
