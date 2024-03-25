function [b_xout,b_yout,theta_mout] = bezier(p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,n)
tList = linspace(0,n,n+1);
b_x = linspace(0,n,n+1);
b_y = linspace(0,n,n+1);
theta_m = linspace(0,n,n+1);

for i = 1:n+1
    tList(i) = tList(i)/n;
    t = tList(i);
    b_x(i) = (1-t)^3*p1x+3*(1-t)^2*t*p2x+3*(1-t)*t^2*p3x+t^3*p4x;
    b_y(i) = (1-t)^3*p1y+3*(1-t)^2*t*p2y+3*(1-t)*t^2*p3y+t^3*p4y;
end
    
for i = 1:n+1
    if i == n+1
        theta_m(i) = theta_m(i-1);
    else
        at = atan2((b_y(i+1)-b_y(i)),(b_x(i+1)-b_x(i)));
        if at>=0
            theta_m(i) = at-pi;
        else
            theta_m(i) = at+pi;
        end
    end
end
%BEZIER Summary of this function goes here
%   Detailed explanation goes here
b_xout = b_x;
b_yout = b_y;
theta_mout = theta_m;
end

