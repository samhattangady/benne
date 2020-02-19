/*
  The basic building blocks of things.
*/
float PI = 3.14159;
vec3 rotate3D(vec3 point, vec3 rotation) {
    vec3 r = rotation;
	mat3 rz = mat3(cos(r.z), -sin(r.z), 0,
                   sin(r.z),  cos(r.z), 0,
                   0,         0,        1);
    mat3 ry = mat3( cos(r.y), 0, sin(r.y),
                    0       , 1, 0       ,
                   -sin(r.y), 0, cos(r.y));
    mat3 rx = mat3(1, 0       , 0        ,
                   0, cos(r.x), -sin(r.x),
                   0, sin(r.x),  cos(r.x));
    return rx * ry * rz * point;
}
float sdfSphere(vec3 position, vec3 center, float radius) {
    return distance(position, center) - radius;
}
float sdfEllipsoid(vec3 position, vec3 center, vec3 radii) {
    position -= center;
    float k0 = length(position/radii);
    float k1 = length(position/(radii*radii));
    return k0*(k0-1.0)/k1;
}
float sdfEllipsoidRotated(vec3 position, vec3 center, vec3 radii, vec3 rotation) {
	position -= center;
    position = rotate3D(position, rotation);
    float k0 = length(position/radii);
    float k1 = length(position/(radii*radii));
    return k0*(k0-1.0)/k1;
}
float sdfPlane( vec3 position, vec4 n ) {
    return dot(position, normalize(n.xyz)) + n.w;
}
float sdfRoundBoxRotated(vec3 position, vec3 center, vec3 box, vec3 rotation, float radius) {
    position -= center;
    position = rotate3D(position, rotation);
    vec3 q = abs(position) - box;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - radius;
}
float dot2(vec2 v) {
	return dot(v, v);
}
vec3 bendSpaceZ (vec3 position, float degree) {
    //position = rotate3D(position, vec3(0.0, PI/2.0, 0.0));
    float k = degree;
    float c = cos(k*position.y);
    float s = sin(k*position.y);
    mat2  m = mat2(c,-s,s,c);
    vec2  q = m*position.xy;
    return vec3(q, position.z);
}
vec4 sdfJoint3DSphere(vec3 position, vec3 start, vec3 rotation, float len, float angle, float thickness) {
    vec3 p = position;
    float l = len;
    float a = angle;
    float w = thickness;
    p -= start;
    p = rotate3D(p, rotation);

    if( abs(a)<0.001 ) {
        return vec4( length(p-vec3(0,clamp(p.y,0.0,l),0))-w, p );
    }

    vec2  sc = vec2(sin(a),cos(a));
    float ra = 0.5*l/a;
    p.x -= ra;
    vec2 q = p.xy - 2.0*sc*max(0.0,dot(sc,p.xy));
    float u = abs(ra)-length(q);
    float d2 = (q.y<0.0) ? dot2( q+vec2(ra,0.0) ) : u*u;
    float s = sign(a);
    return vec4( sqrt(d2+p.z*p.z)-w,
               (p.y>0.0) ? s*u : s*sign(-p.x)*(q.x+ra),
               (p.y>0.0) ? atan(s*p.y,-s*p.x)*ra : (s*p.x<0.0)?p.y:l-p.y,
               p.z );
}
float smin(float d1, float d2, float k) {
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}
float smax(float d1, float d2, float k) {
    float h = max(k-abs(d1-d2),0.0);
    return max(d1, d2) + h*h*0.25/k;
}
