function handles = add_plotter_display_panel(obj,handles,parent)

    layout = uiextras.VBox( 'Parent', parent, 'Spacing', 3 );
    
    handles.graph_axes = axes('Parent',layout);
    
    param_layout = uiextras.Grid( 'Parent', layout, 'Spacing', 3 );
    uicontrol( 'Style', 'text', 'String', 'Label  ', 'Parent', param_layout, ...
               'HorizontalAlignment', 'right' );
    uicontrol( 'Style', 'text', 'String', 'Parameter  ', 'Parent', param_layout, ...
               'HorizontalAlignment', 'right' );
           
    handles.graph_independent_popupmenu = uicontrol( 'Style', 'popupmenu', ...
            'String', {''}, 'Parent', param_layout );
    handles.graph_dependent_popupmenu = uicontrol( 'Style', 'popupmenu', ...
            'String', {''}, 'Parent', param_layout );
        
    uicontrol( 'Style', 'text', 'String', 'Combine  ', 'Parent', param_layout, ...
               'HorizontalAlignment', 'right' );
    uicontrol( 'Style', 'text', 'String', 'Error bars  ', 'Parent', param_layout, ...
               'HorizontalAlignment', 'right' );
    
    handles.graph_grouping_popupmenu = uicontrol( 'Style', 'popupmenu', ...
            'String', {'By Pixel' 'By Region' 'By FOV'}, 'Parent', param_layout );       
    handles.error_type_popupmenu = uicontrol( 'Style', 'popupmenu', ...
            'String', {'Standard Deviation', 'Standard Error', '95% Confidence'}, 'Parent', param_layout );
        
    uicontrol( 'Style', 'text', 'String', 'Display  ', 'Parent', param_layout, ...
               'HorizontalAlignment', 'right' );
    uiextras.Empty( 'Parent', param_layout );
           
    handles.graph_display_popupmenu = uicontrol( 'Style', 'popupmenu', ...
            'String', {'Line' 'Line with Scatter' 'Box Plot'}, 'Parent', param_layout );       
    uiextras.Empty( 'Parent', param_layout );
    
        
    set( param_layout, 'RowSizes', [22,22] );
    set( param_layout, 'ColumnSizes', [90,90,90,90,90,90] );
    
    set( layout, 'Sizes', [-1 70])
    
    
    plate_layout = uiextras.VBox( 'Parent', parent, 'Spacing', 3 );
    
        
    plate_container = uicontainer( 'Parent', plate_layout );
    handles.plate_axes = axes( 'Parent', plate_container );
    
    param_layout = uiextras.Grid( 'Parent', plate_layout, 'Spacing', 3 );
    uicontrol( 'Style', 'text', 'String', 'Parameter  ', 'Parent', param_layout, ...
               'HorizontalAlignment', 'right' );
    handles.plate_param_popupmenu = uicontrol( 'Style', 'popupmenu', ...
            'String', {''}, 'Parent', param_layout );
    
    set( param_layout, 'RowSizes', [22] );
    set( param_layout, 'ColumnSizes', [100,100] );
    
    set( plate_layout, 'Sizes', [-1 70])
    
    
    
    

end