#include "header.h"


// La proc�dure mailleur permet de g�n�rer le maillage global du projet.
// Elle prend pour arguments (pass�s en r�f�rence):
//		- Le nombre et la liste des points (vector<point>). Le num�ro d'un point correspond � sa position dans la liste. Toutes les autres structures ne font que r�f�rence au num�ro et ne stockent pas de points.
//		- Le nombre et la liste des segments (poutres potentielles ou simples ar�tes de plaques). Sont stock�s les num�ros des noeuds (bord et interne).
//		- Le nombre et la liste des rectangles (futures plaques). Sont stock�s les num�ros des �l�ments et des segments de bord.
//		- Le nombre et la liste des faces (futurs �l�ments de plaques).
//		- La longueur caract�ristique d'un �l�ment d'ar�te.

void mailleur(int& nb_points, vector<point>& liste_de_points, int& nb_segs, vector<segment>& liste_de_segments, int& nb_rects, vector<rectangle>& liste_de_rectangles, int& nb_face, vector<face>& liste_de_faces, double taille) {
	
	// R�cup�ration des donn�es "utilisateur"
	// --------------------------------------

	// Encapsulage des donn�es de points, segments et rectangles dans des objets de type "vector".
	// Ceux-ci seront compl�t�s lors de la construction du maillage:
	//		- Pour les segments, num�ros des points internes (hors extr�mit�s)
	//		- Pour les rectangles, num�ros des �l�ments de plaque inclus.

	// Lecture des points

	ifstream file_points("liste_de_points.csv");
	file_points >> nb_points;
	for (int k = 0; k < nb_points; k++) {
		double x, y, z;
		file_points >> x >> y >> z;
		liste_de_points.push_back(point(x, y, z));
	};
	file_points.close();

	// Lecture des segments

	ifstream file_segs("liste_de_segments.csv");
	file_segs >> nb_segs;
	for (int k = 0; k < nb_segs; k++) {
		int p1, p2;
		file_segs >> p1 >> p2;
		liste_de_segments.push_back(segment(p1, p2));
	};
	file_segs.close();

	// Lecture des rectangles

	ifstream file_rects("liste_de_rectangles.csv");
	file_rects >> nb_rects;
	for (int k = 0; k < nb_rects; k++) {
		int l1, l2, l3, l4;
		file_rects >> l1 >> l2 >> l3 >> l4;
		liste_de_rectangles.push_back(rectangle(l1, l2, l3, l4));
	};
	file_rects.close();



	// Maillage des segments
	// ---------------------

	// On parcourt les diff�rents segments, et l'on cr�e les points internes.
	// Ils sont stock�s dans le "vector" de liste de points, et leur num�ro est ajout� dans l'objet "segment" associ�.
	// Ainsi, lors de l'assemblage, pour les poutres, une seule matrice de raideur locale (� rotationner une seule fois) est � cr�er,
	// puisque les �l�ments sont identiques par construction.

	for (int k = 0; k < nb_segs; k++) {
		point pt1 = liste_de_points[liste_de_segments[k].p1];
		point pt2 = liste_de_points[liste_de_segments[k].p2];
		double longueur = (pt1.vectorTo(pt2)).norm();
		int nb_sseg = longueur / taille + 1;
		double nb_sseg_d = nb_sseg;

		for (int j = 1; j < nb_sseg; j++) {
			double taux = j / nb_sseg_d;
			liste_de_points.push_back(point(pt1.interpolTo(pt2, taux)));
			liste_de_segments[k].add_no(nb_points);
			nb_points++;
		};
	};

	// Maillage des rectangles
	// -----------------------

	// On parcourt les diff�rents rectangles, et l'on cr�e les points internes (les points externes ont �t� maill�s pr�c�demment).
	// Ils sont stock�s dans le "vector" de liste de points, leur num�ro est ajout� dans un objet "face". 
	// Cet objet face est lui-m�me stock� dans un "vector" de face de l'objet "rectangle associ�.
	// Ainsi, lors de l'assemblage, pour les plaques, une seule matrice de raideur locale (� rotationner une seule fois) est � cr�er,
	// puisque les �l�ments sont identiques par construction.

	// Pas besoin de v�rifier l'ordre des points dans les lignes d'apr�s Romain, la sortie de l'interface le fait d�j�.

	for (int k = 0; k < nb_rects; k++) {
		segment seg1 = liste_de_segments[liste_de_rectangles[k].l1]; segment seg2 = liste_de_segments[liste_de_rectangles[k].l2];
		segment seg3 = liste_de_segments[liste_de_rectangles[k].l3]; segment seg4 = liste_de_segments[liste_de_rectangles[k].l4];

		int nb_u = seg1.nb; int nb_v = seg4.nb; // Nombre de sous-segments, i.e. nombre d'�l�ments � cr�er (et un de plus que le nombre de points par ligne/colonne).

		point pt1 = liste_de_points[seg1.p1];
		point pt_u = liste_de_points[seg1.interne[0]];
		point pt_v = liste_de_points[seg4.interne[nb_v - 2]];
		vecteur u, v;
		u = pt1.vectorTo(pt_u);
		v = pt1.vectorTo(pt_v);

		// Cr�ation de la permi�re ligne d'�l�ments
		//		Cr�ation de l'�l�ment de premi�re colonne
		liste_de_points.push_back(point(pt1.x + u.dx + v.dx, pt1.y + u.dy + v.dy, pt1.z + u.dz + v.dz));
		liste_de_rectangles[k].add_facette(nb_face);
		liste_de_faces.push_back(face(seg1.p1, seg1.interne[0], nb_points, seg4.interne[nb_v - 2]));
		nb_face++;
		nb_points++;
		//		Cr�ation des �l�ments des (nb_v-2) colonnes internes
		for (int j = 2; j < nb_u; j++) {
			liste_de_points.push_back(point(pt1.x + j * u.dx + v.dx, pt1.y + j * u.dy + v.dy, pt1.z + j * u.dz + v.dz));
			liste_de_rectangles[k].add_facette(nb_face);
			liste_de_faces.push_back(face(seg1.interne[j - 2], seg1.interne[j - 1], nb_points, nb_points - 1));
			nb_face++;
			nb_points++;
		};
		//		Cr�ation de l'�l�ment de derni�re colonne
		liste_de_rectangles[k].add_facette(nb_face);
		liste_de_faces.push_back(face(seg1.interne[nb_u - 2], seg1.p2, seg2.interne[0], nb_points - 1));
		nb_face++;

		// Cr�ation des (nb_u-2) lignes d'�l�ments internes
		for (int i = 2; i < nb_v; i++) {
			//		Cr�ation de l'�l�ment de premi�re colonne
			liste_de_points.push_back(point(pt1.x + u.dx + i * v.dx, pt1.y + u.dy + i * v.dy, pt1.z + u.dz + i * v.dz));
			liste_de_rectangles[k].add_facette(nb_face);
			liste_de_faces.push_back(face(seg4.interne[nb_v - i], nb_points - nb_u + 1, nb_points, seg4.interne[nb_v - i - 1]));
			nb_face++;
			nb_points++;
			//		Cr�ation des �l�ments des (nb_v-2) colonnes internes
			for (int j = 2; j < nb_u; j++) {
				liste_de_points.push_back(point(pt1.x + j * u.dx + i * v.dx, pt1.y + j * u.dy + i * v.dy, pt1.z + j * u.dz + i * v.dz));
				liste_de_rectangles[k].add_facette(nb_face);
				liste_de_faces.push_back(face(nb_points - nb_u, nb_points - nb_u + 1, nb_points, nb_points - 1));
				nb_face++;
				nb_points++;
			};

			//		Cr�ation de l'�l�ment de derni�re colonne
			liste_de_rectangles[k].add_facette(nb_face);
			liste_de_faces.push_back(face(nb_points - nb_u, seg2.interne[i - 2], seg2.interne[i - 1], nb_points - 1));
			nb_face++;
		};


		// Cr�ation de la derni�re ligne d'�l�ments
		//		Cr�ation de l'�l�ment de premi�re colonne
		liste_de_rectangles[k].add_facette(nb_face);
		liste_de_faces.push_back(face(seg4.interne[0], nb_points - nb_u + 1, seg3.interne[nb_u - 2], seg4.p1));
		nb_face++;
		//		Cr�ation des �l�ments des (nb_v-2) colonnes internes
		for (int j = 2; j < nb_u; j++) {
			liste_de_rectangles[k].add_facette(nb_face);
			liste_de_faces.push_back(face(nb_points - nb_u + j - 1, nb_points - nb_u + j, seg3.interne[nb_u - j - 1], seg3.interne[nb_u - j]));
			nb_face++;
		};
		//		Cr�ation de l'�l�ment de derni�re colonne
		liste_de_rectangles[k].add_facette(nb_face);
		liste_de_faces.push_back(face(nb_points - 1, seg2.interne[nb_v - 2], seg3.p1, seg3.interne[0]));
		nb_face++;

	};
};

int main() {

	// D�claration des variables g�n�rales
	// -----------------------------------

	// Concernant la d�finition des points
	int nb_points;
	vector<point> liste_de_points;

	// Concernant la d�finition des segments
	int nb_segs;
	vector<segment> liste_de_segments;

	// Concernant la d�finition des rectangles
	int nb_rects;
	vector<rectangle> liste_de_rectangles;

	// Concernant les �l�ments de plaque
	int nb_face = 0;
	vector<face> liste_de_faces;

	// Taille d'un �l�ment (ligne)
	double taille=1.9;

	// Cr�ation du maillage
	// --------------------

	mailleur(nb_points, liste_de_points, nb_segs, liste_de_segments, nb_rects, liste_de_rectangles, nb_face, liste_de_faces, taille);

	// Contr�le en utilisant Tikz --> �tape provisoire
	// -----------------------------------------------

	ofstream MyFile("sortie_tex.txt");

	for (int k = 0; k < nb_points; k++) {
		MyFile << "\\draw (" << liste_de_points[k].x << "," << liste_de_points[k].y << "," << liste_de_points[k].z << ") node[below] {\\color{blue} " << k << "};"<<endl;
	};

	for (int k = 0; k < nb_face; k++) {
		MyFile << "\\draw ";
		MyFile << "(" << liste_de_points[liste_de_faces[k].p1].x << "," << liste_de_points[liste_de_faces[k].p1].y << "," << liste_de_points[liste_de_faces[k].p1].z << ")--";
		MyFile << "(" << liste_de_points[liste_de_faces[k].p2].x << "," << liste_de_points[liste_de_faces[k].p2].y << "," << liste_de_points[liste_de_faces[k].p2].z << ")--";
		MyFile << "(" << liste_de_points[liste_de_faces[k].p3].x << "," << liste_de_points[liste_de_faces[k].p3].y << "," << liste_de_points[liste_de_faces[k].p3].z << ")--";
		MyFile << "(" << liste_de_points[liste_de_faces[k].p4].x << "," << liste_de_points[liste_de_faces[k].p4].y << "," << liste_de_points[liste_de_faces[k].p4].z << ")--";
		MyFile << "cycle;" << endl;

		MyFile << "\\draw (" << 0.5 * (liste_de_points[liste_de_faces[k].p1].x + liste_de_points[liste_de_faces[k].p3].x) << ",";
		MyFile << 0.5 * (liste_de_points[liste_de_faces[k].p1].y + liste_de_points[liste_de_faces[k].p3].y) << ",";
		MyFile << 0.5 * (liste_de_points[liste_de_faces[k].p1].z + liste_de_points[liste_de_faces[k].p3].z) << ") node[align=center] {" << k << "};" << endl;
	};
	

	return 0;
};