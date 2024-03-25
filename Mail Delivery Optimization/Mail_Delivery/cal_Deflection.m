function [epsilon] = cal_Deflection(x0,y0,theta_m)
%CAL Summary of this function goes here
%   Detailed explanation goes here
m = 3.8e-2; %kg
h_b = 15e-3; % m
k = 0.0289/1000/pi*180; % Nm
mu = 0.4;
N = 5; % num of paper
Area = 125*(0.4e-6)*N; % m^2
E1 = 3470*10^6;% Pa
E0 = 176.2*10^6; % Pa
g = 9.81; % m/s^2
a = sqrt(x0^2+y0^2); % m
l_m = 0.225;%m
feasible = 1;
% x_n = x0-l_m*cos(theta_m);
% y_n = y0-l_m*sin(theta_m);
% if(x_n <= 0 && y_n >=0)
%     feasible  = 0;
%     epsilon = 100;
% end
if feasible
    syms x y
    if a < h_b
        theta_b = -atan2(y0,x0)-pi;
    else
        a = h_b;
        f_circle = x^2 + y^2 - (0.015)^2;
        f_line = cos(theta_m)*y - sin(theta_m)*x + (sin(theta_m)*x0 - cos(theta_m)*y0);
        ANS = solve([f_circle == 0,f_line == 0],[x y]);
        Ans = struct2array(ANS);
        A = double(Ans);
        B = A(2,:);
         if isreal(B)
            if(A(1,1) < A(2,1))
               B(1,1) = A(1,1);
               B(1,2) = A(1,2);
            end
            theta_b = - atan2(B(1,2),B(1,1)) - pi/2 ;
         else
             theta_b = 0;
         end
    end
    fn = (0.5*m*g*h_b*sin(theta_b) + k*theta_b)/a;
    fp = mu*fn;
    F_t = fp*cos(pi-theta_m-theta_b)+fn*sin(pi-theta_m-theta_b);
    F_n = fn*cos(pi-theta_m-theta_b)+fp*sin(pi-theta_m-theta_b);
    Tau = 3*F_n/(2*Area);
    Sigma = abs(F_t/Area + Tau);
    epsilon = Sigma/E1 + (Sigma/E0)^0.58;
end
end

