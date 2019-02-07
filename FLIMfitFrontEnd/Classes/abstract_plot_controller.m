classdef abstract_plot_controller < abstract_display_controller
    
    % Copyright (C) 2013 Imperial College London.
    % All rights reserved.
    %
    % This program is free software; you can redistribute it and/or modify
    % it under the terms of the GNU General Public License as published by
    % the Free Software Foundation; either version 2 of the License, or
    % (at your option) any later version.
    %
    % This program is distributed in the hope that it will be useful,
    % but WITHOUT ANY WARRANTY; without even the implied warranty of
    % MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    % GNU General Public License for more details.
    %
    % You should have received a copy of the GNU General Public License along
    % with this program; if not, write to the Free Software Foundation, Inc.,
    % 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    %
    % This software tool was developed with support from the UK 
    % Engineering and Physical Sciences Council 
    % through  a studentship from the Institute of Chemical Biology 
    % and The Wellcome Trust through a grant entitled 
    % "The Open Microscopy Environment: Image Informatics for Biological Sciences" (Ref: 095931).

    % Author : Sean Warren


    properties
        param_popupmenu;
        result_controller;
        
        param_list;     
        cur_param;   
        ap_lh;
    end
        
    methods

        
        function obj = abstract_plot_controller(handles,plot_handle,param_popupmenu,exports_data)
                       
            obj = obj@abstract_display_controller(handles,plot_handle,exports_data);
            
            obj.plot_handle = plot_handle;
                        
            if nargin >= 3
                obj.param_popupmenu = param_popupmenu;
                set(obj.param_popupmenu,'Callback',@obj.param_select_update);
            else
                obj.param_popupmenu = [];
            end
                                                
            assign_handles(obj,handles);
            
            addlistener(obj.result_controller,'result_updated',@(~,~) EC(@obj.result_update));
            addlistener(obj.result_controller,'result_display_updated',@(~,~) EC(@obj.result_display_update));
           
        end
        
        function plot_fit_update(obj) 
        end
        
        function selection_updated(obj,~,~)
            obj.selected = obj.data_series_list.selected;
            obj.update_display();
        end
        
        
        function update_param_menu(obj,~,~)
            if ~isempty(obj.result_controller.fit_result)
                obj.param_list = obj.result_controller.fit_result.fit_param_list();
                new_list = obj.param_list;
                for i=1:length(obj.param_popupmenu) 
                    old_list = get(obj.param_popupmenu(i),'String')';
                    
                    changed = length(old_list)~=length(new_list) ...
                           || any(~cellfun(@strcmp,old_list,new_list));

                    if changed
                        idx = [];
                        
                        if iscell(old_list)
                            old_val = get(obj.param_popupmenu(i),'Value');
                            old_sel = old_list{old_val};
                            idx = find(strcmp(new_list,old_sel));
                        end
                        
                        set(obj.param_popupmenu(i),'String',new_list);
                        
                        if ~isempty(idx)
                            set(obj.param_popupmenu(i),'Value',idx);
                        elseif get(obj.param_popupmenu(i),'Value') > length(obj.param_list)
                            set(obj.param_popupmenu(i),'Value',1);
                        end
                        
                        obj.param_select_update();    
                    end          
                end
            end
            
        end
        
        function param_select_update(obj,src,evt)
            % Get parameters from potentially multiple popupmenus
            val = get(obj.param_popupmenu,'Value');
            if iscell(val)
                val = cell2mat(val);
            end
            obj.cur_param = val;
            
            obj.update_display();
        end
        
        function lims_update(obj)
            obj.update_display();
        end
        
        function result_update(obj)
            obj.update_param_menu();
            obj.plot_fit_update();
            obj.update_display();
            %obj.ap_lh = addlistener(obj.result_controller.fit_result,'cur_lims','PostSet',@(~,~) EC(@obj.lims_update));
        end
        
        function result_display_update(obj)
            obj.update_display();
        end
                
        function mapped_data = apply_colourmap(obj,data,param,lims)
            
            cscale = obj.colourscale(param);
            
            m=2^8;
            data = data - lims(1);
            data = data / (lims(2) - lims(1));
            data(data > 1) = 1;
            data(data < 0) = 0;
            data = data * m + 1;
            data(isnan(data)) = 0;
            data = int32(data);
            cmap = cscale(m);
            cmap = [ [1,1,1]; cmap];
            
            mapped_data = ind2rgb(data,cmap);
            
        end
        
        function cscale = colourscale(obj,param)
            
            param = obj.result_controller.fit_result.params{param};
            invert = obj.result_controller.invert_colormap;
            
            if contains(param,'_I_') || strcmp(param,'I') 
                cscale = @gray;
            elseif invert && (contains(param,'tau') || contains(param,'theta'))
                cscale = @inv_jet;
            else
                cscale = @jet;
            end

            %cscale = @newmap;
            

            
        end
        
        function im_data = plot_figure(obj,h,hc,dataset,param,merge,text)

            if isempty(obj.result_controller.fit_result) || obj.result_controller.fit_result.binned
                return
            end
            
            f = obj.result_controller;

            intensity = f.get_intensity(dataset,param,'result');
            im_data = f.get_image(dataset,param,'result');

            
            cscale = obj.colourscale(param);

            lims = f.get_cur_lims(param);
            I_lims = f.get_cur_intensity_lims(param);
            if ~merge
                im=colorbar_flush(h,hc,im_data,isnan(intensity),lims,cscale,text);
            else
                im=colorbar_flush(h,hc,im_data,[],lims,cscale,text,intensity,I_lims);
            end
            

            if get(h,'Parent')==obj.plot_handle
                set(im,'uicontextmenu',obj.contextmenu);
            end
            
        end
       
    end
    
end