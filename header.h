#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <cmath>

using namespace Eigen;
using namespace std;

#include <chrono>
using namespace std::chrono;

// Création des classes
// --------------------

// Classe de vecteur

class vecteur {
public:
    double dx, dy, dz;
    vecteur() {};
    vecteur(double i, double j, double k) {
        dx = i;
        dy = j;
        dz = k;
    };
    double norm() {
        return sqrt(dx * dx + dy * dy + dz * dz);
    };
    vecteur normalize() {
        vecteur vec;
        double norme = sqrt(dx * dx + dy * dy + dz * dz);
        vec.dx = dx / norme; vec.dy = dy / norme; vec.dz = dz / norme;
        return vec;
    };
    double Pscalar(vecteur vec) {
        return vec.dx * dx + vec.dy * dy + vec.dz * dz;
    };
    vecteur Pvector(vecteur vec) {
        return vecteur(dy * vec.dz - dz * vec.dy, dz * vec.dx - dx * vec.dz, dx * vec.dy - dy * vec.dx);
    };
};

// Classe de point

class point {
public:
    double x, y, z;
    point() {};
    point(double i, double j, double k) {
        x = i; y = j; z = k;
    }
    vecteur vectorTo(point pt) {
        return vecteur(pt.x - x, pt.y - y, pt.z - z);
    };
    vecteur vectorFrom(point pt) {
        return vecteur(x - pt.x, y - pt.y, z - pt.z);
    };
    point interpolTo(point pt, double taux) {
        return point(x + taux * (pt.x - x), y + taux * (pt.y - y), z + taux * (pt.z - z));
    }
};

// Classe de segment

class segment {
public:
    int p1, p2, nb;
    // "nb" correspond au nombre de sous-segments (nombre de points moins un).
    vector<int> interne;
    segment() {};
    segment(int p1_in, int p2_in) {
        p1 = p1_in; p2 = p2_in; nb = 1;
    };
    void add_no(int no) {
        interne.push_back(no);
        nb++;
    };
};

// Classe de rectangle
class rectangle {
public:
    int l1, l2, l3, l4, nb;
    vector<int> facettes;
    rectangle() {};
    rectangle(int l1_in, int l2_in, int l3_in, int l4_in) {
        l1 = l1_in; l2 = l2_in; l3 = l3_in; l4 = l4_in; nb = 0;
    };
    void add_facette(int no) {
        facettes.push_back(no);
        nb++;
    };
};

class face {
public:
    int p1, p2, p3, p4;
    face() {};
    face(int p1_in, int p2_in, int p3_in, int p4_in) {
        p1 = p1_in; p2 = p2_in; p3 = p3_in; p4 = p4_in;
    };

    // Cette méthode renvoie la matrice de passage de la base locale dans la base globale
    Matrix3d local2global(vector<point>& lst) {
        point pt1, pt2, pt3, pt4;
        pt1 = lst[p1]; pt2 = lst[p2]; pt3 = lst[p3]; pt4 = lst[p4];

        vecteur ex = (pt1.vectorTo(pt2)).normalize();
        vecteur ey = (pt1.vectorTo(pt4)).normalize();
        vecteur ez = ex.Pvector(ey);

        vecteur eX(1, 0, 0);
        vecteur eY(0, 1, 0);
        vecteur eZ(0, 0, 1);

        Matrix3d m3_0;

        m3_0 << ex.Pscalar(eX), ey.Pscalar(eX), ez.Pscalar(eX),
            ex.Pscalar(eY), ey.Pscalar(eY), ez.Pscalar(eY),
            ex.Pscalar(eZ), ey.Pscalar(eZ), ez.Pscalar(eZ);
        return m3_0;
    };

};

typedef Triplet<double> T;