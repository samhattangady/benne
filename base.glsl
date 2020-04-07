vec3 calcNormal(vec3 p) {
    // We calculate the normal by finding the gradient of the field at the
    // point that we are interested in. We can find the gradient by getting
    // the difference in field at that point and a point slighttly away from it.
    const float h = 0.0001;
    return normalize( vec3(
        			       -distanceField(p).x+ distanceField(p+vec3(h,0.0,0.0)).x,
                           -distanceField(p).x+ distanceField(p+vec3(0.0,h,0.0)).x,
                           -distanceField(p).x+ distanceField(p+vec3(0.0,0.0,h)).x
    				 ));
}

vec4 raymarch(vec3 direction, vec3 start) {
    // We need to cast out a ray in the given direction, and see which is
    // the closest object that we hit. We then move forward by that distance,
    // and continue the same process. We terminate when we hit an object
    // (distance is very small) or at some predefined distance.
    float far = 15.0;
    vec3 pos = start;
    float d = 0.0;
    vec4 obj = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i=0; i<100; i++) {
    	obj = distanceField(pos);
        float dist = obj.x;
        pos += dist*direction;
        d += dist;
        if (dist < 0.01) {
        	break;
        }
        if (d > far) {
        	break;
        }
    }
    return vec4(d, obj.yzw);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalise and set center to origin.
    vec2 p = fragCoord/iResolution.xy;
    p -= 0.5;
    p.y *= iResolution.y/iResolution.x;

    float mouseX = ((iMouse.x/iResolution.x)-0.5) * 2.0 * 3.14159/2.0;
    // mouseX = -0.0;
    // mouseX = 0.4*sin(iTime/3.6);
    vec3 cameraPosition = vec3(0.0, 0.0, -3.0);
    vec3 planePosition = vec3(p, 1.0) + cameraPosition;

    mat2 camRotate = mat2(cos(mouseX), -sin(mouseX), sin(mouseX), cos(mouseX));
    cameraPosition.xz = camRotate * cameraPosition.xz;
    planePosition.xz = camRotate * planePosition.xz;

    float yRotate = -((iMouse.y/iResolution.y)-0.5) * 2.0 * 3.14159/2.0;
    //yRotate = 0.2*sin(iTime/4.2);
    camRotate = mat2(cos(yRotate), -sin(yRotate), sin(yRotate), cos(yRotate));
    cameraPosition.yz = camRotate * cameraPosition.yz;
    planePosition.yz = camRotate * planePosition.yz;

    vec3 lookingDirection = (planePosition - cameraPosition);

    // This was fun to sort out, but is it the best way?
    float lightTime = iTime/3.0;
    float multiplier = -1.0 + (step(-0.0, sin(lightTime*3.14159)) *2.0);
    float parabola = (4.0 * fract(lightTime) * (1.0-fract(lightTime)));
    float lightX = multiplier*parabola *-1.2;
    vec3 lightPoint = normalize(vec3(lightX, 1.0, -1.0));
    vec3 lightFacing = lightPoint - vec3(0.0);
    // lightFacing = vec3(1.0, 1.0, -0.3) - vec3(0.0);

    // raymarch to check for colissions.
    vec4 obj = raymarch(lookingDirection, planePosition);
    float dist = obj.x;
    vec3 color = vec3(0.01);
    if (dist < 15.0) {
        vec3 normal = calcNormal(planePosition+ dist*lookingDirection);
        float light = dot(lightFacing, normal);
        light = max(light, 0.0);
        if (obj.y < 1.5) {
            // skin
        	color = vec3(0.505, 0.205, 0.105);
            color += 0.3* smoothstep(0.1, 1.0, light);
        } else if (obj.y < 2.5) {
            //eyes
        	color = vec3(0.65, 0.65, 0.95);
            color += 0.1 * smoothstep(0.5, 1.0, light);
        } else if (obj.y < 3.5) {
        	color = vec3(0.21, 0.21, 0.41);
            color += 0.7 * smoothstep(0.4, 1.0, pow(light, 5.0));
        } else if (obj.y < 4.5) {
        	color = vec3(0.01);
            color += 0.5 * smoothstep(0.4, 1.0, pow(light, 5.0));
        } else if (obj.y < 5.5) {
            // eyebrows
        	color = vec3(0.05, 0.02, 0.01);
            color += 0.05 * smoothstep(0.4, 1.0, pow(light, 5.0));
        } else if (obj.y < 6.5) {
            // mouth
        	color = vec3(0.15, 0.02, 0.01);
            color += 0.1 * smoothstep(0.4, 1.0, pow(light, 5.0));
        }
        float edge = dot(lookingDirection, normal);
        if ((edge > -0.1) && (edge < 0.1))
            color = vec3(1.0);
    }
    // gamma correction
    color = pow( color, vec3(1.0/2.2) );
    fragColor = vec4(color, 1.0);
}

