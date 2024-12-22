gl_Position = u_View.ProjectionMatrix * u_View.ViewMatrix * vec4(a_Position, 1.0);
    
v_WorldPosition = a_Position;