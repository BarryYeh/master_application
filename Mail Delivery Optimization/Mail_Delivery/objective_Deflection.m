function epsilon = objective_Deflection(p)

p2x = p(1);
p2y = p(2);
p3x = p(3);
p3y = p(4);
p1y = p(5);

% set parameter
n = 100;% N_B number of sample points
p1x = 0;
p4x = -0.1; 
p4y = -0.017;
l_m = 0.225;% Mail Length

[b_x, b_y,theta_m] = bezier(p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,n);

parfor i = 1:n+1
    x_n = b_x(i)+l_m*cos(theta_m(i));
    y_n = b_y(i)+l_m*sin(theta_m(i));
    % soft contraint
    if(x_n <= 0 && y_n >=0 ||x_n <=0 && y_n<=-0.015|| theta_m(i)<(-pi/20) || theta_m(i)>pi/2)
    % soft contraint release
    %if((x_n <= 0 && y_n >=0)||x_n <=0 && y_n<=-0.015|| theta_m(i)>pi/2)
        epsilon(i) = 1000*n; % At least 1000 times penalty
    else
        [epsilon(i)] = cal_Deflection(b_x(i),b_y(i),theta_m(i));
    end
end
epsilon = sum(epsilon)/(n+1); % Average Epsilon
end

