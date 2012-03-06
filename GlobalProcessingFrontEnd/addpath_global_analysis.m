function addpath_global_analysis()

    if ~isdeployed
        thisdir = fileparts( mfilename( 'fullpath' ) );
        addpath( thisdir,...
                [thisdir filesep 'Classes'],...
                [thisdir filesep 'GUIDEInterfaces'],...
                [thisdir filesep 'GeneratedFiles'],...
                [thisdir filesep 'HelperFunctions'],...
                [thisdir filesep 'HelperFunctions' filesep 'RawDataFunctions'],...
                [thisdir filesep 'HelperFunctions' filesep 'GUILayout-v1p8'],...
                [thisdir filesep 'HelperFunctions' filesep 'GUILayout-v1p8' filesep 'Patch'],...
                [thisdir filesep 'HelperFunctions' filesep 'GUILayout-v1p8' filesep 'layoutHelp'],...
                [thisdir filesep '..' filesep 'GlobalProcessingLibrary' filesep 'Libraries']);
    end

end