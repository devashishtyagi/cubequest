/*
 * structure.h
 *
 *  Created on: Nov 8, 2011
 *      Author: devashish
 */

class location{
public:
	float v[3];
	location();
	location(float, float, float);
	location add(location);
	void assign(location);
	bool isLess(location);
};

class wall{
public:
	location min, max;
	wall(location, location);
	void generateRect(location*);
};

class obstacle{
public:
	location min, max, curr_min, curr_max;
	float vel[3];
	float t, curr_time;
	obstacle(location, location, location, float);
	void check(float);
	void generateRect(location*);
};

class holes{
public:
	location pos;
	float radius;
	holes(location, float);
};

class flame{
public:
	location pos;
	flame(location);
};

location::location(){
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
}

location::location(float x, float y, float z){
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

location location::add(location b){
	location a;
	for(int i=0; i<3; i++)
		a.v[i] = v[i] + b.v[i];
	return a;
}

void location::assign(location a){
	for(int i=0; i<3; i++)
		v[i] = a.v[i];
}

bool location::isLess(location a){
	bool ans = true;
	for(int i=0; i<3; i++){
		if (v[i] > a.v[i])
			ans = false;
	}
	return ans;
}

wall::wall(location a, location b){
	min.assign(a);
	max.assign(b);
}

void wall::generateRect(location* vertexData){
	for(int j=0; j<6; j=j+2){
		for(int k=0; k<3; k++){
			if(j/2 == k){
				(vertexData[j]).v[k] = min.v[k];
				(vertexData[j+1]).v[k] = min.v[k];
			}
			else{
				(vertexData[j]).v[k] = min.v[k];
				(vertexData[j+1]).v[k] = max.v[k];
			}
		}
	}
	for(int j=0; j<6; j=j+2){
		for(int k=0; k<3; k++){
			if(j/2 == k){
				(vertexData[6+j]).v[k] = max.v[k];
				(vertexData[6+j+1]).v[k] = max.v[k];
			}
			else{
				(vertexData[6+j]).v[k] = min.v[k];
				(vertexData[6+j+1]).v[k] = max.v[k];
			}
		}
	}
}

obstacle::obstacle(location a, location b, location c, float duration){
	min.assign(a);
	max.assign(b);
	curr_min.assign(a);
	curr_max.assign(b);
	vel[0] = c.v[0]/duration;
	vel[1] = c.v[1]/duration;
	vel[2] = c.v[2]/duration;
	t = duration;
	curr_time = 0.0f;
}

void obstacle::generateRect(location* vertexData){
	for(int j=0; j<6; j=j+2){
		for(int k=0; k<3; k++){
			if(j/2==k){
				(vertexData[j]).v[k] = curr_min.v[k];
				(vertexData[j+1]).v[k] = curr_min.v[k];
			}
			else{
				(vertexData[j]).v[k] = curr_min.v[k];
				(vertexData[j+1]).v[k] = curr_max.v[k];
			}
		}
	}
	for(int j=0; j<6; j=j+2){
		for(int k=0; k<3; k++){
			if(j/2==k){
				(vertexData[6+j]).v[k] = curr_max.v[k];
				(vertexData[6+j+1]).v[k] = curr_max.v[k];
			}
			else{
				(vertexData[6+j]).v[k] = curr_min.v[k];
				(vertexData[6+j+1]).v[k] = curr_max.v[k];
			}
		}
	}
}

void obstacle::check(float dtime){
	for(int i=0; i<3; i++){
		curr_min.v[i] += vel[i]*dtime;
		curr_max.v[i] += vel[i]*dtime;
	}
	curr_time += dtime;
	if (curr_time > t){
		vel[0] = -vel[0];
		vel[1] = -vel[1];
		vel[2] = -vel[2];
		curr_time = 0.0f;
	}
}

holes::holes(location p, float r){
	pos.assign(p);
	radius = r;
}

flame::flame(location p){
	pos.assign(p);
}
