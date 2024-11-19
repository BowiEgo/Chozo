gl_Position = u_Camera.ProjectionMatrix * u_Camera.ViewMatrix * vec4(a_Position, 1.0);
    
v_WorldPosition = a_Position;