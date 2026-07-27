#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <utility>
#include <cmath>
#include <limits>
#include "myVector3D.h"

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}

myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++)
		if (vertices[i])
			delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++)
		if (halfedges[i])
			delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++)
		if (faces[i])
			delete faces[i];

	vector<myVertex *> empty_vertices;
	vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges;
	halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;
	faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	int err_twin_null = 0;
	int err_twin_back = 0;
	int err_twin_self = 0;
	int err_twin_vert = 0;
	int err_next_null = 0;
	int err_next_prev = 0;
	int err_face_null = 0;
	int err_face_loop = 0;
	int err_origin = 0;

	for (unsigned int i = 0; i < halfedges.size(); i++)
	{
		myHalfedge *h = halfedges[i];
		if (h == NULL)
			continue;

		if (h->twin == NULL)
			err_twin_null++;
		else
		{
			if (h->twin == h)
				err_twin_self++;
			if (h->twin->twin != h)
				err_twin_back++;
			if (h->next != NULL && h->twin->source != h->next->source)
				err_twin_vert++;
		}

		if (h->next == NULL || h->prev == NULL)
			err_next_null++;
		else
		{
			if (h->next->prev != h)
				err_next_prev++;
			if (h->prev->next != h)
				err_next_prev++;
		}

		if (h->adjacent_face == NULL)
			err_face_null++;
		else if (h->next != NULL && h->next->adjacent_face != h->adjacent_face)
			err_face_loop++;
	}

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		myVertex *v = vertices[i];
		if (v == NULL)
			continue;
		if (v->originof == NULL || v->originof->source != v)
			err_origin++;
	}

	int total = err_twin_null + err_twin_back + err_twin_self + err_twin_vert +
				err_next_null + err_next_prev + err_face_null + err_face_loop +
				err_origin;

	cout << "---- checkMesh ----\n";
	cout << "vertices: " << vertices.size()
		 << "  halfedges: " << halfedges.size()
		 << "  faces: " << faces.size() << "\n";
	if (err_twin_null)
		cout << "  halfedges without twin (boundary?): " << err_twin_null << "\n";
	if (err_twin_self)
		cout << "  twin == self: " << err_twin_self << "\n";
	if (err_twin_back)
		cout << "  twin->twin != self: " << err_twin_back << "\n";
	if (err_twin_vert)
		cout << "  twin->source != next->source: " << err_twin_vert << "\n";
	if (err_next_null)
		cout << "  missing next/prev pointer: " << err_next_null << "\n";
	if (err_next_prev)
		cout << "  next/prev not reciprocal: " << err_next_prev << "\n";
	if (err_face_null)
		cout << "  halfedge without adjacent face: " << err_face_null << "\n";
	if (err_face_loop)
		cout << "  face loop has inconsistent adjacent_face: " << err_face_loop << "\n";
	if (err_origin)
		cout << "  vertex originof inconsistent: " << err_origin << "\n";

	if (total == 0 && err_twin_null == 0)
		cout << "  OK: all half-edge invariants hold.\n";
	else if (total - err_twin_null == 0)
		cout << "  OK (open mesh): only boundary edges lack a twin.\n";
	else
		cout << "  FAILED: " << (total - err_twin_null) << " violation(s).\n";
	cout << "-------------------\n";
}

bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge **hedges;

	ifstream fin(filename);
	if (!fin.is_open())
	{
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge *> twin_map;
	map<pair<int, int>, myHalfedge *>::iterator it;

	while (getline(fin, s))
	{
		stringstream myline(s);
		if (!(myline >> t))
			continue;
		if (t == "g")
		{
		}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			cout << "v " << x << " " << y << " " << z << endl;

			myPoint3D *p = new myPoint3D(x, y, z);
			myVertex *v = new myVertex();
			v->point = p;
			vertices.push_back(v);
		}
		else if (t == "mtllib")
		{
		}
		else if (t == "usemtl")
		{
		}
		else if (t == "s")
		{
		}
		else if (t == "f")
		{
			faceids.clear();
			while (myline >> u)
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);

			if (faceids.size() < 3)
				continue;

			hedges = new myHalfedge *[faceids.size()];
			for (unsigned int i = 0; i < faceids.size(); i++)
				hedges[i] = new myHalfedge();

			myFace *f = new myFace();
			f->adjacent_halfedge = hedges[0];

			for (unsigned int i = 0; i < faceids.size(); i++)
			{
				int iplusone = (i + 1) % faceids.size();
				int iminusone = (i - 1 + faceids.size()) % faceids.size();

				hedges[i]->source = vertices[faceids[i]];
				hedges[i]->adjacent_face = f;
				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];

				it = twin_map.find(make_pair(faceids[iplusone], faceids[i]));
				if (it != twin_map.end())
				{
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				}
				else
					twin_map[make_pair(faceids[i], faceids[iplusone])] = hedges[i];

				if (vertices[faceids[i]]->originof == NULL)
					vertices[faceids[i]]->originof = hedges[i];

				halfedges.push_back(hedges[i]);
			}

			delete[] hedges;
			faces.push_back(f);
		}
	}

	checkMesh();
	normalize();

	return true;
}

void myMesh::computeNormals()
{
	for (unsigned int i = 0; i < faces.size(); i++)
		faces[i]->computeNormal();

	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i]->computeNormal();
}

void myMesh::normalize()
{
	if (vertices.size() < 1)
		return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X)
			tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X)
			tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y)
			tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y)
			tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z)
			tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z)
			tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		   ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		   zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::subdivisionCatmullClark()
{
	if (faces.empty()) return;
	map<myFace *, myPoint3D *> facePoint;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		myHalfedge *start = faces[i]->adjacent_halfedge;
		myPoint3D sum(0, 0, 0);
		int n = 0;
		myHalfedge *h = start;
		do {
			sum += *(h->source->point);
			n++;
			h = h->next;
		} while (h != start);
		facePoint[faces[i]] = new myPoint3D(sum.X / n, sum.Y / n, sum.Z / n);
	}
	map<myHalfedge *, myPoint3D *> edgePoint;
	for (unsigned int i = 0; i < halfedges.size(); i++)
	{
		myHalfedge *h = halfedges[i];
		if (edgePoint.count(h)) continue;
		myPoint3D *a = h->source->point;
		myPoint3D *b = h->next->source->point;
		if (h->twin != NULL)
		{
			myPoint3D *f1 = facePoint[h->adjacent_face];
			myPoint3D *f2 = facePoint[h->twin->adjacent_face];
			edgePoint[h] = new myPoint3D((a->X + b->X + f1->X + f2->X) / 4.0,
				(a->Y + b->Y + f1->Y + f2->Y) / 4.0,
				(a->Z + b->Z + f1->Z + f2->Z) / 4.0);
			edgePoint[h->twin] = edgePoint[h];
		}
		else
		{
			edgePoint[h] = new myPoint3D((a->X + b->X) / 2.0,
				(a->Y + b->Y) / 2.0,
				(a->Z + b->Z) / 2.0);
		}
	}
	map<myVertex *, myPoint3D> faceSum, edgeSum;
	map<myVertex *, int> valence;
	map<myVertex *, vector<myVertex *> > voisinsBord;
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		faceSum[vertices[i]] = myPoint3D(0, 0, 0);
		edgeSum[vertices[i]] = myPoint3D(0, 0, 0);
		valence[vertices[i]] = 0;
		voisinsBord[vertices[i]] = vector<myVertex *>();
	}
	for (unsigned int i = 0; i < halfedges.size(); i++)
	{
		myHalfedge *h = halfedges[i];
		if (h->twin == NULL)
		{
			myVertex *a = h->source;
			myVertex *b = h->next->source;
			voisinsBord[a].push_back(b);
			voisinsBord[b].push_back(a);
		}
	}
	for (unsigned int i = 0; i < halfedges.size(); i++)
	{
		myHalfedge *h = halfedges[i];
		myVertex *v = h->source;
		myPoint3D *fp = facePoint[h->adjacent_face];
		myPoint3D *a = v->point;
		myPoint3D *b = h->next->source->point;
		faceSum[v] += *fp;
		edgeSum[v].X += (a->X + b->X) / 2.0;
		edgeSum[v].Y += (a->Y + b->Y) / 2.0;
		edgeSum[v].Z += (a->Z + b->Z) / 2.0;
		valence[v]++;
	}
	map<myVertex *, myPoint3D> newPos;
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		myVertex *v = vertices[i];
		if (voisinsBord[v].size() == 2)
		{
			myPoint3D *P = v->point;
			myPoint3D *A = voisinsBord[v][0]->point;
			myPoint3D *B = voisinsBord[v][1]->point;
			newPos[v] = myPoint3D(
				0.75 * P->X + 0.125 * (A->X + B->X),
				0.75 * P->Y + 0.125 * (A->Y + B->Y),
				0.75 * P->Z + 0.125 * (A->Z + B->Z)
			);
			continue;
		}
		else if (voisinsBord[v].size() == 1)
		{
			newPos[v] = *(v->point);
			continue;
		}
		int n = valence[v];
		if (n == 0)
		{
			newPos[v] = *(v->point);
			continue;
		}
		myPoint3D F = faceSum[v];
		myPoint3D R = edgeSum[v];
		myPoint3D *P = v->point;
		newPos[v] = myPoint3D((F.X / n + 2.0 * R.X / n + (n - 3.0) * P->X) / n,
			(F.Y / n + 2.0 * R.Y / n + (n - 3.0) * P->Y) / n,
			(F.Z / n + 2.0 * R.Z / n + (n - 3.0) * P->Z) / n);
	}
	map<myFace *, myVertex *> fvert;
	map<myHalfedge *, myVertex *> evert;
	vector<myVertex *> newVertices;
	vector<myHalfedge *> newHalfedges;
	vector<myFace *> newFaces;
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		*(vertices[i]->point) = newPos[vertices[i]];
		vertices[i]->originof = NULL;
		newVertices.push_back(vertices[i]);
	}
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		myVertex *v = new myVertex();
		v->point = facePoint[faces[i]];
		fvert[faces[i]] = v;
		newVertices.push_back(v);
	}
	for (unsigned int i = 0; i < halfedges.size(); i++)
	{
		myHalfedge *h = halfedges[i];
		if (evert.count(h)) continue;
		myVertex *v = new myVertex();
		v->point = edgePoint[h];
		evert[h] = v;
		if (h->twin != NULL) evert[h->twin] = v;
		newVertices.push_back(v);
	}
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		myFace *f = faces[i];
		myVertex *fv = fvert[f];
		vector<myHalfedge *> loop;
		myHalfedge *h = f->adjacent_halfedge;
		do {
			loop.push_back(h);
			h = h->next;
		} while (h != f->adjacent_halfedge);
		int n = (int)loop.size();
		for (int k = 0; k < n; k++)
		{
			myHalfedge *cur = loop[k];
			myHalfedge *prevh = loop[(k - 1 + n) % n];
			myVertex *c0 = cur->source;
			myVertex *c1 = evert[cur];
			myVertex *c2 = fv;
			myVertex *c3 = evert[prevh];
			myFace *nf = new myFace();
			newFaces.push_back(nf);
			myHalfedge *e0 = new myHalfedge();
			myHalfedge *e1 = new myHalfedge();
			myHalfedge *e2 = new myHalfedge();
			myHalfedge *e3 = new myHalfedge();
			e0->source = c0; e1->source = c1; e2->source = c2; e3->source = c3;
			e0->next = e1; e1->next = e2; e2->next = e3; e3->next = e0;
			e0->prev = e3; e1->prev = e0; e2->prev = e1; e3->prev = e2;
			e0->adjacent_face = nf; e1->adjacent_face = nf;
			e2->adjacent_face = nf; e3->adjacent_face = nf;
			nf->adjacent_halfedge = e0;
			if (c0->originof == NULL) c0->originof = e0;
			if (c1->originof == NULL) c1->originof = e1;
			if (c2->originof == NULL) c2->originof = e2;
			if (c3->originof == NULL) c3->originof = e3;
			newHalfedges.push_back(e0);
			newHalfedges.push_back(e1);
			newHalfedges.push_back(e2);
			newHalfedges.push_back(e3);
		}
	}
	map<pair<myVertex *, myVertex *>, myHalfedge *> twin_map;
	for (unsigned int i = 0; i < newHalfedges.size(); i++)
	{
		myHalfedge *e = newHalfedges[i];
		myVertex *src = e->source;
		myVertex *dst = e->next->source;
		pair<myVertex *, myVertex *> backward(dst, src);
		if (twin_map.count(backward))
		{
			myHalfedge *tw = twin_map[backward];
			e->twin = tw;
			tw->twin = e;
		}
		else
		{
			twin_map[pair<myVertex *, myVertex *>(src, dst)] = e;
		}
	}
	for (unsigned int i = 0; i < halfedges.size(); i++) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) delete faces[i];
	vertices = newVertices;
	halfedges = newHalfedges;
	faces = newFaces;
	checkMesh();
	computeNormals();
}

void myMesh::simplify()
{
	if (faces.empty() || halfedges.empty() || vertices.size() < 3)
		return;

	int V = static_cast<int>(vertices.size());
	for (int i = 0; i < V; i++)
		vertices[i]->index = i;

	vector<myPoint3D> pos;
	pos.reserve(V);
	for (int i = 0; i < V; i++)
		pos.push_back(myPoint3D(vertices[i]->point->X, vertices[i]->point->Y, vertices[i]->point->Z));

	vector<int> tris;
	for (unsigned int fi = 0; fi < faces.size(); fi++)
	{
		myFace *f = faces[fi];
		if (f == NULL || f->adjacent_halfedge == NULL)
			continue;
		myHalfedge *h0 = f->adjacent_halfedge;
		myHalfedge *h = h0->next;
		while (h != NULL && h->next != h0 && h->next != NULL)
		{
			tris.push_back(h0->source->index);
			tris.push_back(h->source->index);
			tris.push_back(h->next->source->index);
			h = h->next;
		}
	}
	if (tris.empty())
		return;

	vector<int> parent(V);
	for (int i = 0; i < V; i++)
		parent[i] = i;
	struct UF
	{
		static int find(vector<int> &p, int x)
		{
			int root = x;
			while (p[root] != root)
				root = p[root];
			while (p[x] != root)
			{
				int nx = p[x];
				p[x] = root;
				x = nx;
			}
			return root;
		}
	};

	int collapse_count = static_cast<int>(faces.size() * 0.1);
	if (collapse_count < 1)
		collapse_count = 1;

	int ntris = static_cast<int>(tris.size()) / 3;
	for (int iter = 0; iter < collapse_count; iter++)
	{
		double best_len2 = std::numeric_limits<double>::max();
		int best_a = -1, best_b = -1;
		for (int t = 0; t < ntris; t++)
		{
			int r[3] = {
				UF::find(parent, tris[3 * t + 0]),
				UF::find(parent, tris[3 * t + 1]),
				UF::find(parent, tris[3 * t + 2])};
			if (r[0] == r[1] && r[1] == r[2])
				continue;
			for (int k = 0; k < 3; k++)
			{
				int a = r[k];
				int b = r[(k + 1) % 3];
				if (a == b)
					continue;
				double dx = pos[a].X - pos[b].X;
				double dy = pos[a].Y - pos[b].Y;
				double dz = pos[a].Z - pos[b].Z;
				double len2 = dx * dx + dy * dy + dz * dz;
				if (len2 < best_len2)
				{
					best_len2 = len2;
					best_a = a;
					best_b = b;
				}
			}
		}

		if (best_a < 0 || best_b < 0)
			break;

		pos[best_a] = myPoint3D(
			(pos[best_a].X + pos[best_b].X) * 0.5,
			(pos[best_a].Y + pos[best_b].Y) * 0.5,
			(pos[best_a].Z + pos[best_b].Z) * 0.5);
		parent[best_b] = best_a;
	}

	vector<int> outTris;
	outTris.reserve(tris.size());
	map<int, int> rootToNew;
	vector<myPoint3D> outPos;
	for (int t = 0; t < ntris; t++)
	{
		int r0 = UF::find(parent, tris[3 * t + 0]);
		int r1 = UF::find(parent, tris[3 * t + 1]);
		int r2 = UF::find(parent, tris[3 * t + 2]);
		if (r0 == r1 || r1 == r2 || r2 == r0)
			continue;

		int idx[3] = {r0, r1, r2};
		for (int k = 0; k < 3; k++)
		{
			map<int, int>::iterator it = rootToNew.find(idx[k]);
			int nid;
			if (it == rootToNew.end())
			{
				nid = static_cast<int>(outPos.size());
				rootToNew[idx[k]] = nid;
				outPos.push_back(pos[idx[k]]);
			}
			else
				nid = it->second;
			outTris.push_back(nid);
		}
	}

	if (outTris.empty() || outPos.size() < 3)
		return;

	clear();

	vector<myVertex *> nv(outPos.size(), NULL);
	for (unsigned int i = 0; i < outPos.size(); i++)
	{
		myVertex *v = new myVertex();
		v->point = new myPoint3D(outPos[i].X, outPos[i].Y, outPos[i].Z);
		nv[i] = v;
		vertices.push_back(v);
	}

	map<pair<int, int>, myHalfedge *> twin_map;
	for (unsigned int t = 0; t + 2 < outTris.size(); t += 3)
	{
		int id[3] = {outTris[t], outTris[t + 1], outTris[t + 2]};
		myFace *f = new myFace();
		myHalfedge *e[3];
		for (int k = 0; k < 3; k++)
			e[k] = new myHalfedge();
		for (int k = 0; k < 3; k++)
		{
			e[k]->source = nv[id[k]];
			e[k]->adjacent_face = f;
			e[k]->next = e[(k + 1) % 3];
			e[k]->prev = e[(k + 2) % 3];
			if (nv[id[k]]->originof == NULL)
				nv[id[k]]->originof = e[k];
			halfedges.push_back(e[k]);
		}
		f->adjacent_halfedge = e[0];
		faces.push_back(f);

		for (int k = 0; k < 3; k++)
		{
			int a = id[k];
			int b = id[(k + 1) % 3];
			map<pair<int, int>, myHalfedge *>::iterator it = twin_map.find(make_pair(b, a));
			if (it != twin_map.end())
			{
				e[k]->twin = it->second;
				it->second->twin = e[k];
			}
			else
				twin_map[make_pair(a, b)] = e[k];
		}
	}

	checkMesh();
	computeNormals();
}

void myMesh::simplify(myVertex *)
{
	// Not used for the shortest-edge simplifier (keep simple).
}

void myMesh::revolution()
{
	vector<myPoint3D> profile;
	profile.push_back(myPoint3D(0.15, -0.20, 0.0));
	profile.push_back(myPoint3D(0.20, -0.25, 0.0));
	profile.push_back(myPoint3D(0.30, -0.25, 0.0));
	profile.push_back(myPoint3D(0.35, 0.00, 0.0));
	profile.push_back(myPoint3D(0.90, 0.0, 0.0));

	clear();

	const int slices = 20;
	const double two_pi = 6.28318530717958647692;
	const int profile_count = static_cast<int>(profile.size());

	vector<vector<myVertex *>> grid(slices, vector<myVertex *>(profile_count, NULL));
	for (int s = 0; s < slices; s++)
	{
		double angle = (two_pi * static_cast<double>(s)) / static_cast<double>(slices);
		double c = cos(angle);
		double si = sin(angle);
		for (int i = 0; i < profile_count; i++)
		{
			const myPoint3D &p = profile[i];
			myVertex *v = new myVertex();
			v->point = new myPoint3D(c * p.X + si * p.Z, p.Y, -si * p.X + c * p.Z);
			vertices.push_back(v);
			grid[s][i] = v;
		}
	}

	map<pair<myVertex *, myVertex *>, myHalfedge *> twin_map;
	for (int s = 0; s < slices; s++)
	{
		int sn = (s + 1) % slices;
		for (int i = 0; i < profile_count - 1; i++)
		{
			myVertex *v0 = grid[s][i];
			myVertex *v1 = grid[sn][i];
			myVertex *v2 = grid[sn][i + 1];
			myVertex *v3 = grid[s][i + 1];

			myFace *f = new myFace();
			myHalfedge *e0 = new myHalfedge();
			myHalfedge *e1 = new myHalfedge();
			myHalfedge *e2 = new myHalfedge();
			myHalfedge *e3 = new myHalfedge();

			e0->source = v0;
			e1->source = v1;
			e2->source = v2;
			e3->source = v3;

			e0->adjacent_face = f;
			e1->adjacent_face = f;
			e2->adjacent_face = f;
			e3->adjacent_face = f;

			e0->next = e1;
			e1->next = e2;
			e2->next = e3;
			e3->next = e0;

			e0->prev = e3;
			e1->prev = e0;
			e2->prev = e1;
			e3->prev = e2;

			if (v0->originof == NULL)
				v0->originof = e0;
			if (v1->originof == NULL)
				v1->originof = e1;
			if (v2->originof == NULL)
				v2->originof = e2;
			if (v3->originof == NULL)
				v3->originof = e3;

			f->adjacent_halfedge = e0;
			faces.push_back(f);
			halfedges.push_back(e0);
			halfedges.push_back(e1);
			halfedges.push_back(e2);
			halfedges.push_back(e3);

			myHalfedge *face_edges[4] = {e0, e1, e2, e3};
			for (int k = 0; k < 4; k++)
			{
				myVertex *a = face_edges[k]->source;
				myVertex *b = face_edges[k]->next->source;
				pair<myVertex *, myVertex *> rev_key = make_pair(b, a);
				map<pair<myVertex *, myVertex *>, myHalfedge *>::iterator it = twin_map.find(rev_key);
				if (it != twin_map.end())
				{
					face_edges[k]->twin = it->second;
					it->second->twin = face_edges[k];
				}
				else
					twin_map[make_pair(a, b)] = face_edges[k];
			}
		}
	}
}

void myMesh::triangulate()
{
	vector<myFace *> original_faces = faces;
	for (unsigned int i = 0; i < original_faces.size(); i++)
		triangulate(original_faces[i]);
}

// return false if already triangle, true otherwise.
bool myMesh::triangulate(myFace *f)
{
	const double eps = 1e-10;
	int n = 0;
	myHalfedge *e = f->adjacent_halfedge;
	do
	{
		n++;
		e = e->next;
	} while (e != f->adjacent_halfedge);
	if (n == 3)
		return false;

	vector<myHalfedge *> hedges(n);
	vector<myVertex *> verts(n);
	e = f->adjacent_halfedge;
	for (int i = 0; i < n; i++)
	{
		hedges[i] = e;
		verts[i] = e->source;
		e = e->next;
	}

	myVector3D faceN(0, 0, 0);
	for (int i = 0; i < n; i++)
	{
		myPoint3D *a = verts[i]->point, *b = verts[(i + 1) % n]->point;
		faceN.dX += (a->Y - b->Y) * (a->Z + b->Z);
		faceN.dY += (a->Z - b->Z) * (a->X + b->X);
		faceN.dZ += (a->X - b->X) * (a->Y + b->Y);
	}
	if (faceN.length() < eps)
		return false;

	vector<int> nxt(n), prv(n);
	for (int i = 0; i < n; i++)
	{
		nxt[i] = (i + 1) % n;
		prv[i] = (i - 1 + n) % n;
	}

	int remaining = n;
	int cur = 0;
	while (remaining > 3)
	{
		bool found = false;
		int startIdx = cur;
		do
		{
			int p = prv[cur];
			int nx = nxt[cur];

			myVector3D v1 = *verts[cur]->point - *verts[p]->point;
			myVector3D v2 = *verts[nx]->point - *verts[cur]->point;
			if (v1.crossproduct(v2) * faceN > 0)
			{
				bool isEar = true;
				int test = nxt[nx];
				while (test != p)
				{
					myVector3D c0 = (*verts[cur]->point - *verts[p]->point).crossproduct(*verts[test]->point - *verts[p]->point);
					myVector3D c1 = (*verts[nx]->point - *verts[cur]->point).crossproduct(*verts[test]->point - *verts[cur]->point);
					myVector3D c2 = (*verts[p]->point - *verts[nx]->point).crossproduct(*verts[test]->point - *verts[nx]->point);
					if (c0 * faceN > eps && c1 * faceN > eps && c2 * faceN > eps)
					{
						isEar = false;
						break;
					}
					test = nxt[test];
				}
				if (isEar)
				{
					myHalfedge *d_in = new myHalfedge();
					myHalfedge *d_out = new myHalfedge();
					d_in->source = verts[nx];
					d_out->source = verts[p];
					d_in->twin = d_out;
					d_out->twin = d_in;
					halfedges.push_back(d_in);
					halfedges.push_back(d_out);
					myFace *nf = new myFace();
					faces.push_back(nf);
					nf->adjacent_halfedge = hedges[p];
					hedges[p]->next = hedges[cur];
					hedges[cur]->next = d_in;
					d_in->next = hedges[p];
					hedges[p]->prev = d_in;
					hedges[cur]->prev = hedges[p];
					d_in->prev = hedges[cur];
					hedges[p]->adjacent_face = nf;
					hedges[cur]->adjacent_face = nf;
					d_in->adjacent_face = nf;
					hedges[p] = d_out;
					nxt[p] = nx;
					prv[nx] = p;
					remaining--;
					cur = nx;
					found = true;
					break;
				}
			}
			cur = nxt[cur];
		} while (cur != startIdx);
		if (!found)
			break;
	}

	int i0 = cur;
	int i1 = nxt[i0];
	int i2 = nxt[i1];
	f->adjacent_halfedge = hedges[i0];
	hedges[i0]->next = hedges[i1];
	hedges[i1]->next = hedges[i2];
	hedges[i2]->next = hedges[i0];
	hedges[i0]->prev = hedges[i2];
	hedges[i1]->prev = hedges[i0];
	hedges[i2]->prev = hedges[i1];
	hedges[i0]->adjacent_face = f;
	hedges[i1]->adjacent_face = f;
	hedges[i2]->adjacent_face = f;
	return true;
}
