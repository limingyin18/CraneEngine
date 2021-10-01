//https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky
//https://www.shadertoy.com/view/3dBSDW

#version 450 core

layout (location = 1) in vec2 texFrag;

layout (location = 0) out vec4 colorFragOut;

layout (push_constant) uniform constants
{
	vec4 pos;
	mat4 projView;
}camera;

layout(set = 0, binding = 0) uniform  SceneData{   
    vec4 fogColor;
	vec4 fogDistances;
	vec4 ambientColor;
	vec4 sunlightDirection;
	vec4 sunlightColor;
} sceneData;


#define PI 3.14159265359
#define USE_HENYEY 1
#define SWITCH_COLOR 1

#define u_res iResolution
#define u_time iTime

vec3 sunDir = -sceneData.sunlightDirection.xyz;
float sunPower = 20.;
const float earthRadius = 6360e3;
const float atmosphereRadius = 6420e3;
const int numSamples = 16;
//光线与大气层顶端交点到采样点的采样数量
#if SWITCH_COLOR
const int numSamplesLIght = 8;
const float hR = 7994.;//rayleigh
const float hM = 1200.;//mie
const vec3 betaR = vec3(5.5e-6,13.0e-6,22.4e-6);//rayleigh
const vec3 betaM = vec3(21e-6);//mie
#else
const int numSamplesLIght = 8;
const float hR = 7994.;//rayleigh
const float hM = 1200.;//mie
const vec3 betaR = vec3(1.5e-6,40.0e-6,7.4e-6);//rayleigh
const vec3 betaM = vec3(21e-6);//mie
#endif

struct ray
{
	vec3 origin;
    vec3 direction;
};
struct sphere
{
	vec3 center;
    float radius;
}; 
//表示大气层的球
const sphere atmosphere = sphere(vec3(0.,0.,0.),atmosphereRadius);
bool raySphereIntersect(const in ray r,const in sphere s,inout float t0,inout float t1)
{
	vec3 oc = r.origin-s.center;
    float a = dot(r.direction,r.direction);
    float b = dot(oc,r.direction);
    float c = dot(oc,oc)-s.radius*s.radius;
    float discriminant = b*b-(a*c);
    if(discriminant>0.0)
    {
        float tmp = sqrt(discriminant);
        t0 = (-b-tmp)/a;
        t1 = (-b+tmp)/a;
        return true;
    }
    return false;
}

//绕x轴的旋转矩阵
mat3 rotate_around_x(const in float degrees)
{
    float angle = radians(degrees);
    float _sin = sin(angle);
    float _cos = cos(angle);
    return mat3(1.,0.,0.,
                0.,_cos,-_sin,
                0.,_sin,_cos);
}

ray getPrimaryRay(in vec2 camLocalPoint,inout vec3 camOrigin,inout vec3 camLookAt)
{
	vec3 fwd = normalize(camLookAt-camOrigin);
    vec3 up = vec3(0.,1.,0.);
    vec3 right = cross(up,fwd);
    up = cross(fwd,right);
    ray r = ray(camOrigin,normalize(fwd+camLocalPoint.x*right+camLocalPoint.y*up));
    return r;
}

//----------------phase function----------------
float rayleighPhaseFunc(float mu)
{
	return 3.*(1.+mu*mu)/(16.*PI);
}

const float g = 0.76;
float henyeyGreensteinPhaseFunc(float mu)
{
	return (1.-g*g)/((4.*PI)*pow(1.+g*g-2.*g*mu,1.5));
}

const float k = 1.55*g-0.55*(g*g*g);
float schlickPhaseFunc(float mu)
{
	return (1.-k*k)/(4.*PI*(1.+k*mu)*(1.+k*mu));
}
//----------------phase function----------------

//计算光学深度
bool getSunLight(const in ray r,inout float opticalDepthR,inout float opticalDepthM)
{
	float t0,t1;
    raySphereIntersect(r,atmosphere,t0,t1);
    float marchPos = 0.;
    float marchStep = t1/float(numSamplesLIght);
    for(int i = 0;i<numSamplesLIght;i++)
    {
        //相邻两个采样点的中点
    	vec3 s =r.origin+r.direction*(marchPos+0.5*marchStep);
        float height = length(s)-earthRadius;
        if(height<0.)
            return false;
        opticalDepthR += exp(-height/hR)*marchStep;
        opticalDepthM += exp(-height/hM)*marchStep;
        marchPos += marchStep;
    }
    return true;
}

vec3 getIncidentLight(const in ray r)
{
	float t0,t1;
    if(!raySphereIntersect(r,atmosphere,t0,t1))
    {
    	return vec3(0.);
    }
	float marchStep = t1/float(numSamples);
    float mu = dot(r.direction,sunDir);
    float phaseR = rayleighPhaseFunc(mu);
    float phaseM = 
#if USE_HENYEY
    henyeyGreensteinPhaseFunc(mu);
#else
    schlickPhaseFunc(mu);
#endif
    float opticalDepthR = 0.;
    float opticalDepthM = 0.;
    
    vec3 sumR = vec3(0.);
    vec3 sumM = vec3(0.);
    float marchPos = 0.;
    for(int i=0;i<numSamples;i++)
    {
    	vec3 s = r.origin+r.direction*(marchPos+0.5*marchStep);
        float height = length(s)-earthRadius;
        //计算光学深度累加和
        float hr = exp(-height/hR)*marchStep;
        float hm = exp(-height/hM)*marchStep;
        opticalDepthR += hr;
        opticalDepthM += hm;
        
        ray lightRay = ray(s,sunDir);
        float opticalDepthLightR = 0.;
        float opticalDepthLightM = 0.;
        bool bOverGround = getSunLight(lightRay,opticalDepthLightR,opticalDepthLightM);
    	if(bOverGround)
        {
        	vec3 t = betaR*(opticalDepthR+opticalDepthLightR)+
                betaM*1.1*(opticalDepthM+opticalDepthLightM);
            vec3 attenuation = exp(-t);
            sumR += hr*attenuation;
            sumM += hm*attenuation;
        }
        marchPos += marchStep;
    }
    return sunPower*(sumR*phaseR*betaR+sumM*phaseM*betaM);
}

void main()
{
    vec2 p = (2.*texFrag-1.);

    vec3 color = vec3(0.);

    vec3 from = vec3(0.,earthRadius+1.,0.);
    vec3 lookat = vec3(0.,earthRadius+1.5,-1.);
    ray r = getPrimaryRay(p,from,lookat);
    if(dot(r.direction,vec3(0.,1.,0.))>0.)
        color = getIncidentLight(r);
    else
        color = vec3(0.33,0.33,0.33);

    colorFragOut = vec4(color,1.);
}