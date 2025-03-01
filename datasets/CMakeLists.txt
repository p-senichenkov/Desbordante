add_custom_target(make-input-data-dir ALL)
add_custom_command(
	TARGET make-input-data-dir
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/
	POST_BUILD
)

add_custom_target( unZip ALL )
add_custom_command(
	TARGET unZip
	COMMAND ${CMAKE_COMMAND} -E tar xzf
		${CMAKE_SOURCE_DIR}/datasets/datasets.zip
	POST_BUILD
	WORKING_DIRECTORY
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/
	COMMENT "Unpacking datasets.zip"
	VERBATIM
)

add_dependencies(unZip make-input-data-dir)

# TODO(senichenkov): if there will be a lot of cropped datasets, it'd be better to
# read 'em from some kind of list
# FIXME(senichenkov): `head`, `cat` and `cut` are not cross-platform
add_custom_target( cropDatasets ALL
	DEPENDS
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/adult5attr50.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/CIPublicHighway200k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/CIPublicHighway10attr.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/CIPublicHighway20attr50k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa100k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa25attr1kk.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa500k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa600k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa900k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/mushroomAndCIPublicHighway800.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/neighbors120k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/neighbors130k.csv
)
add_custom_command(
	OUTPUT
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/adult5attr50.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/CIPublicHighway200k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/CIPublicHighway10attr.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/CIPublicHighway20attr50k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa100k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa25attr1kk.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa500k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa600k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/iowa900k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/mushroomAndCIPublicHighway800.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/neighbors120k.csv
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/neighbors130k.csv
	COMMAND cut -d';' -f -5 adult.csv | head -n51 > adult5attr50.csv
	COMMAND head -n200001 CIPublicHighway.csv > CIPublicHighway200k.csv
	COMMAND cut -d, -f -10 CIPublicHighway.csv > CIPublicHighway10attr.csv
	COMMAND cut -d, -f -20 CIPublicHighway.csv | head -n50001 > CIPublicHighway20attr50k.csv
	COMMAND head -n100001 iowa1kk.csv > iowa100k.csv
	# Actually, in some lines only 24 attributes are selected, beacuse "Store Location" field
	# contains comma, but it's enough for testing purposes
	COMMAND cut -d, -f -25 iowa1kk.csv > iowa25attr1kk.csv
	COMMAND head -n500001 iowa1kk.csv > iowa500k.csv
	COMMAND head -n600001 iowa1kk.csv > iowa600k.csv
	COMMAND head -n900001 iowa1kk.csv > iowa900k.csv
	COMMAND paste cfd_data/mushroom.csv CIPublicHighway.csv -d, | cut -d, -f -27 | head -n801 > mushroomAndCIPublicHighway800.csv
	COMMAND cat neighbors100k.csv neighbors100k.csv | head -n120001 > neighbors120k.csv
	COMMAND cat neighbors100k.csv neighbors100k.csv | head -n130001 > neighbors130k.csv
	WORKING_DIRECTORY
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/input_data/
)

add_dependencies(cropDatasets unZip)
