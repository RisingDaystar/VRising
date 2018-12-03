
//
//BSD 3-Clause License
//
//Copyright (c) 2017-2018, Alessandro Berti
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//* Neither the name of the copyright holder nor the names of its
//  contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//


#ifndef _VOPERATORS_H
#define _VOPERATORS_H

#include "VAureaNox.h"

using namespace vnx;

struct vop_union : public VOperator {
	float blend_factor = 0.0f;
	vop_union(std::string idv, float bfactor, std::initializer_list<VNode*> chs) : VOperator(idv, chs), blend_factor(bfactor){}
	vop_union(std::string idv, std::initializer_list<VNode*> chs) : VOperator(idv, chs), blend_factor(0.0f) {}

	inline const char* type(){return "vop_union";}

	inline void eval(const vec3f& p,VResult& res) {
		if (childs.empty()) { return; }
		ygl::vec3f ep = p;
		if (scale != one3f) {
			ep = transform_point_inverse(_frame, p) / scale;
			ep = transform_point(_frame, ep);
		}
		VResult vre;
		childs[0]->eval(p,res);
		auto vdist = res.vdist;
		auto vmat = res.vmat;
		auto vsur = res.vsur;
		if (blend_factor < ygl::epsf) {
			for (int i = 1; i < childs.size(); i++) {
				childs[i]->eval(ep,vre);

				if (std::abs(vre.dist) < std::abs(res.dist)) { res = vre; }
				if(vre.vdist<vdist) {
                    vdist = vre.vdist;
                    ///DIFFERENZIAZIONE DEL MATERIALE VOLUMETRICO : DA TESTARE -> + TODO per gli altri operatori
                    if(vdist>res.dist){vmat = vre.vmat;vsur = vre.vsur;}
                    else{vmat = res.mat;vsur = res.sur;}
                }
			}
		}
		else {
            auto sdist = res.dist;
            auto vsdist = res.vdist;
			for (int i = 1; i < childs.size(); i++) {
				childs[i]->eval(ep,vre);
				sdist = smin(sdist, vre.dist, blend_factor);
				vsdist = smin(vsdist,vre.vdist, blend_factor);


				if (std::abs(vre.dist) < std::abs(res.dist)) { res = vre; }
				if(vre.vdist<vdist) {vdist = vre.vdist;vmat = vre.vmat;vsur = vre.vsur;}
			}
            res.dist = sdist;
            vdist = vsdist;
		}
		res.vdist = vdist;
		res.vmat = vmat;
		res.wor_pos = p;


		auto dspm = eval_displacement(ep);
		auto sfct = min_element(scale);
		res.dist += dspm;
		res.dist *= sfct;

		res.vdist += dspm;
		res.vdist *= sfct;
	};
};


struct vop_intersection : public VOperator {
	float blend_factor = 0.0f;
	bool preserve_material = false;

	vop_intersection(std::string idv, float bfactor, bool pmaterial, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(bfactor), preserve_material(pmaterial) {}
	vop_intersection(std::string idv, float bfactor, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(bfactor), preserve_material(false) {}
	vop_intersection(std::string idv, bool pmaterial, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(0.0f), preserve_material(pmaterial) {}
	vop_intersection(std::string idv, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(0.0f), preserve_material(false) {}

	inline const char* type(){return "vop_intersection";}

	inline void eval(const vec3f& p,VResult& res) {
		if (childs.empty()) { return; }
		ygl::vec3f ep = p;
		if (scale != one3f) {
			ep = transform_point_inverse(_frame, p) / scale;
			ep = transform_point(_frame, ep);
		}
		VResult vre;
		VMaterial* mtl = nullptr;
		VMaterial* vmtl = nullptr;
        childs[0]->eval(p,res);
        mtl = res.mat;
        vmtl = res.vmat;
        auto vdist = res.vdist;
        auto vmat = res.vmat;
        auto vsur = res.vsur;
		if (blend_factor < ygl::epsf) {
			for (int i = 1; i < childs.size(); i++) {
				childs[i]->eval(ep,vre);
				if (vre.dist > res.dist) { res = vre; }
				if(vre.vdist>vdist) {vdist = vre.vdist;vmat = vre.vmat;vsur = vre.vsur;}
			}
		}
		else {
            auto sdist = res.dist;
            auto vsdist = res.vdist;
			for (int i = 1; i < childs.size(); i++) {
				childs[i]->eval(ep,vre);
				sdist = smax(sdist, vre.dist, blend_factor);
				vsdist = smax(vsdist, vre.vdist, blend_factor);

				if (vre.dist > res.dist) { res = vre; }
				if(vre.vdist>vdist) {vdist = vre.vdist;vmat = vre.vmat;vsur = vre.vsur;}
			}
			res.dist = sdist;
            vdist = vsdist;
		}
        res.vdist = vdist;
		res.wor_pos = p;

		auto dspm = eval_displacement(ep);
		auto sfct = min_element(scale);
		res.dist += dspm;
		res.dist *= sfct;

		res.vdist += dspm;
		res.vdist *= sfct;
		if (preserve_material) { res.mat = mtl;res.vmat = vmtl; }
		else{res.vmat = vmat;}
	};
};

struct vop_subtraction : public VOperator {
	float blend_factor = 0.0f;
	bool preserve_material = false;

	vop_subtraction(std::string idv, float bfactor, bool pmaterial, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(bfactor), preserve_material(pmaterial) {}
	vop_subtraction(std::string idv, float bfactor, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(bfactor), preserve_material(false) {}
	vop_subtraction(std::string idv, bool pmaterial, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(0.0f), preserve_material(pmaterial) {}
	vop_subtraction(std::string idv, std::initializer_list<VNode*> chs) : VOperator(idv,chs), blend_factor(0.0f), preserve_material(false) {}

	inline const char* type(){return "vop_subtraction";}

	inline void eval(const vec3f& p,VResult& res) {
		if (childs.empty()) { return; }
		ygl::vec3f ep = p;
		if (scale != one3f) {
			ep = transform_point_inverse(_frame, p) / scale;
			ep = transform_point(_frame, ep);
		}
		VResult vre;
		VMaterial* mtl = nullptr;
		VMaterial* vmtl = nullptr;
        childs[0]->eval(p,res);
        mtl = res.mat;
        vmtl = res.vmat;
        auto vdist = res.vdist;
        auto vmat = res.vmat;
        auto vsur = res.vsur;
		if (blend_factor < ygl::epsf) {
			for (int i = 1; i < childs.size(); i++) {
				childs[i]->eval(ep,vre);

				if (-vre.dist > res.dist) { res = vre;res.dist = -vre.dist; }
				if(-vre.dist>vdist) {vdist = -vre.vdist;vmat = vre.vmat;vsur = vre.vsur;}
			}
		}
		else {
            auto sdist = res.dist;
            auto vsdist = res.vdist;
			for (int i = 1; i < childs.size(); i++) {
				childs[i]->eval(ep,vre);
				sdist = smax(sdist,-vre.dist, blend_factor);
				vsdist = smax(vsdist,-vre.vdist, blend_factor);

				if (-vre.dist > res.dist) { res = vre;res.dist = -vre.dist; }
				if(-vre.vdist>vdist) {vdist = -vre.vdist;vmat = vre.vmat;vsur = vre.vsur;}
			}
            res.dist = sdist;
            vdist = vsdist;
		}
		res.vdist = vdist;
		res.wor_pos = p;
		auto dspm = eval_displacement(ep);
		auto sfct = min_element(scale);
		res.dist += dspm;
		res.dist *= sfct;

		res.vdist += dspm;
		res.vdist *= sfct;
		if (preserve_material) { res.mat = mtl;res.vmat = vmtl; }
		else{res.vmat = vmat;}
	};
};

struct vop_twist : public VOperator {
	e_axis axis = X;
	float amount = 1.0f;

	vop_twist(std::string idv, VNode* chs) : VOperator(idv, { chs }), axis(X), amount(1.0f) {}
	vop_twist(std::string idv, float am, VNode* chs) : VOperator(idv, { chs }), axis(X), amount(am) {}
	vop_twist(std::string idv, e_axis ax, VNode* chs) : VOperator(idv, { chs }), axis(ax), amount(1.0f) {}
	vop_twist(std::string idv, e_axis ax, float am, VNode* chs) : VOperator(idv, { chs }), axis(ax), amount(am) {}

	inline const char* type(){return "vop_twist";}

	virtual void eval(const ygl::vec3f& p,VResult& res) {
        if (childs.empty()) { return; }
		ygl::vec3f ep = transform_point_inverse(_frame, p) / scale;
        //ep = transform_point(_frame, ep);

		VResult vre;
		if (axis == X) {
			float c = cos(amount*ep.x);
			float s = sin(amount*ep.x);
			ygl::mat2f  m = ygl::mat2f{{ c, -s }, { s, c }};
			auto mres = m*ygl::vec2f{ ep.y,ep.z };
			ygl::vec3f  q = ygl::vec3f{mres.x, mres.y, ep.x};
			q = ygl::transform_point(_frame, q);
			childs[0]->eval(q,res);
		}
		else if (axis == Y) {
			float c = cos(amount*ep.y);
			float s = sin(amount*ep.y);
			ygl::mat2f  m = ygl::mat2f{{ c, -s }, { s, c }};
			auto mres = m*ygl::vec2f{ ep.x,ep.z };
			ygl::vec3f  q = ygl::vec3f{mres.x, mres.y, ep.y};
			q = ygl::transform_point(_frame, q);
			childs[0]->eval(q,res);
		}
		else if (axis == Z) {
			float c = cos(amount*ep.z);
			float s = sin(amount*ep.z);
			ygl::mat2f  m = ygl::mat2f{{ c, -s }, { s, c }};
			auto mres = m*ygl::vec2f{ ep.x,ep.y };
			ygl::vec3f  q = ygl::vec3f{mres.x, mres.y, ep.z};
			q = ygl::transform_point(_frame, q);
			childs[0]->eval(q,res);
		}

		res.wor_pos = p;
		res.dist *= (1.0 / 3.0f);
		res.vdist *= (1.0 / 3.0f);
		auto dspm = eval_displacement(ep);
		auto sfct = min_element(scale);
		res.dist += dspm;
		res.dist *= sfct;

		res.vdist += dspm;
		res.vdist *= sfct;
	}
};

struct vop_repeat : public VOperator {
    ygl::vec3f cells = {1.0f,1.0f,1.0f};

	vop_repeat(std::string idv, VNode* chs) : VOperator(idv, { chs }), cells({1.0f,1.0f,1.0f}) {}
	vop_repeat(std::string idv,const vec3f& cs, VNode* chs) : VOperator(idv, { chs }), cells(cs) {}

	inline const char* type(){return "vop_repeat";}

	virtual void eval(const ygl::vec3f& p,VResult& res) {
	    ygl::vec3f ep = p;
        if(scale!=one3f){
            ep = transform_point_inverse(_frame, p) / scale;
            ep = transform_point(_frame, ep);
        }

        vec3f mpc = gl_mod(ep,cells)-(0.5f*cells);
        childs[0]->eval(mpc,res);

		res.wor_pos = p;
		auto dspm = eval_displacement(ep);
		auto sfct = min_element(scale);
		res.dist += dspm;
		res.dist *= sfct;

		res.vdist += dspm;
		res.vdist *= sfct;
	}
};

struct vop_invert : public VOperator {
	e_axis axis = X;
	float amount = 5.0f;

	vop_invert(std::string idv, VNode* chs) : VOperator(idv, { chs }), axis(X), amount(5.0f) {}
	vop_invert(std::string idv, VNode* chs, e_axis ax) : VOperator(idv, { chs }), axis(ax), amount(5.0f) {}
	vop_invert(std::string idv, VNode* chs, e_axis ax, float am) : VOperator(idv, { chs }), axis(ax), amount(am) {}

	inline const char* type(){return "vop_invert";}

	virtual void eval(const ygl::vec3f& p,VResult& res) {
	    ygl::vec3f ep = p;
        if(scale!=one3f){
            ep = transform_point_inverse(_frame, p) / scale;
            ep = transform_point(_frame, ep);
        }
		childs[0]->eval(ep,res);
		res.wor_pos = p;

		auto dspm = eval_displacement(ep);
		auto sfct = min_element(scale);
		res.dist += dspm;
		res.dist *=-1;
		res.dist *= sfct;

		res.vdist += dspm;
		res.vdist *=-1;
		res.vdist *= sfct;
	}
};



#endif