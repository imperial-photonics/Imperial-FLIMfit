function [n_chan, chan_info] = get_channels(file)

    %> Determine what channels are available in a file

    if (nargin < 1)
        %selects the folder and a file
        %directory = uigetdir;
        [file,path] = uigetfile('*.*');

        % Check that user didn't cancel
        if (file == 0)
            return
        end

        [~,name,ext] = fileparts(file);
        file = [PathName file];
    else
        [PathName,name,ext] = fileparts(file);
    end


    %cd(PathName);
    switch ext

        % .tif files %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        case '.tif'
            n_chan = 1;
            chan_info = {'tif data'};

         % .png files %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
         case '.png'

            n_chan = 1;
            chan_info = {'png data'};

         % .sdt files %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
         case '.sdt'

            %setting 3rd arg to false indicates that no data is to be
            %returned
            dummy_channel = 1; %any no 
            [ImData Delays, chan_info] =loadBHfileusingmeasDescBlock(file, dummy_channel, false);

            n_chan = length(chan_info);


         % .asc files %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%     
         case '.asc'

             n_chan = 1;
             chan_info = {'asc data'};


          % .txt files %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
          case '.txt'
             tcspc = 1;

             fid = fopen(file);

             header_data = cell(0,0);
             textl = fgetl(fid);
             while ~isempty(textl)
                 first = sscanf(textl,'%f\t');
                 if isempty(first) % if it's not a number skip line
                     header_data{end+1} =  textl;
                     textl = fgetl(fid);
                 else 
                     textl = [];
                 end                 
             end
             
             n_header_lines = length(header_data);
             
             header_title = cell(1,n_header_lines);
             header_info = cell(1,n_header_lines);
             
             n_chan = 0;
             for i=1:n_header_lines
                 parts = regexp(header_data{i},'\s+','split');
                 header_title{i} = parts{1};
                 header_info{i} = parts(2:end);
                 n_chan = length(header_info{i})-1;
             end

             chan_info = cell(1,n_chan);
             
             for i=1:n_chan
                 for j=1:n_header_lines
                     chan_info{i} = [chan_info{i} header_info{j}{i} ', '];
                 end
                 chan_info{i} = chan_info{i}(1:(end-2));
             end



        case '.irf'        % Yet another F%^^ing format (for Labview this time)
            n_chan = 1;
            chan_info = {'irf data'};




        otherwise 

            errordlg('Not a .recognised file type!','File Error');
    end
    

end