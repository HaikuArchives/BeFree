add_subdirectory(private)
add_subdirectory(kernel)
add_subdirectory(support)
add_subdirectory(storage)
add_subdirectory(xml)
add_subdirectory(app)
add_subdirectory(render)
add_subdirectory(interface)
add_subdirectory(net)
add_subdirectory(add-ons)

# Build libroot.
set(root_sources
	kernel/Debug.cpp
	kernel/area.cpp
	kernel/image.cpp
	kernel/locker.cpp
	kernel/os.cpp
	kernel/port.cpp
	kernel/semaphore.cpp
	kernel/thread.cpp
	kernel/timefuncs.cpp
	support/List.cpp
	support/String.cpp
	support/StringArray.cpp
	support/SimpleLocker.cpp
	storage/Path.cpp
)
link_libraries(root rt dl)
add_library(root SHARED ${root_sources})

# Build libbe.
set(be_sources
	private/Token.cpp

	support/List.cpp
	support/String.cpp
	support/StringArray.cpp
	support/SimpleLocker.cpp
	support/Locker.cpp
	support/Archivable.cpp
	support/ByteOrder.c
	support/DataIO.cpp

	storage/Path.cpp
	storage/Entry.cpp
	storage/Directory.cpp
	storage/Node.cpp
	storage/Volume.cpp
	storage/VolumeRoster.cpp
	storage/File.cpp
	storage/FindDirectory.cpp
	storage/FilePanel.cpp

	xml/SimpleXmlParser.cpp

	app/Message.cpp
	app/MessageQueue.cpp
	app/Handler.cpp
	app/Looper.cpp
	app/Messenger.cpp
	app/MessageRunner.cpp
	app/MessageFilter.cpp
	app/Invoker.cpp
	app/Clipboard.cpp
	app/Cursor.cpp
	app/Application.cpp

	render/LineGenerator.cpp
	render/ArcGenerator.cpp
	render/Render.cpp
	render/Pixmap.cpp

	interface/layout/LayoutContainer.cpp
	interface/layout/LayoutItem.cpp
	interface/layout/LayoutForm.cpp
	interface/directfb/etk-application.cpp
	interface/directfb/etk-window.cpp
	interface/directfb/etk-pixmap.cpp
	interface/directfb/etk-drawing.cpp
	interface/directfb/etk-dfb-font.cpp
	interface/Alert.cpp
	interface/Bitmap.cpp
	interface/Box.cpp
	interface/Button.cpp
	interface/CheckBox.cpp
	interface/ColorControl.cpp
	interface/Control.cpp
	interface/Font.cpp
	interface/GraphicsDefs.cpp
	interface/LimitedView.cpp
	interface/ListItem.cpp
	interface/ListView.cpp
	interface/MenuBar.cpp
	interface/Menu.cpp
	interface/MenuField.cpp
	interface/MenuItem.cpp
	interface/OutlineListView.cpp
	interface/Point.cpp
	interface/Polygon.cpp
	interface/PopUpMenu.cpp
	interface/RadioButton.cpp
	interface/Rect.cpp
	interface/Region.cpp
	interface/Screen.cpp
	interface/ScrollBar.cpp
	interface/ScrollView.cpp
	interface/StatusBar.cpp
	interface/StringView.cpp
	interface/TabView.cpp
	interface/TextControl.cpp
	interface/TextEditable.cpp
	interface/TextView.cpp
	interface/Theme.cpp
	interface/ToolTip.cpp
	interface/View.cpp
	interface/Window.cpp

	net/NetAddress.cpp
	net/NetBuffer.cpp
	net/NetDebug.cpp
	net/NetEndpoint.cpp

	add-ons/font/FontEngine.cpp
	add-ons/font/FreeType2.cpp
	add-ons/graphics/GraphicsEngine.cpp
	add-ons/theme/ThemeEngine.cpp
	add-ons/theme/DefaultTheme.cpp
)
include_directories(${FREETYPE_INCLUDE_DIRS} ${DIRECTFB_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS} ${DIRECTFB_LIBRARY_DIRS})
link_libraries(be root ${FREETYPE_LIBRARIES} ${DIRECTFB_LIBRARIES})
add_library(be SHARED ${be_sources})

# Install libraries
install(
	TARGETS root be
	DESTINATION ${SYSTEM_LIB_DIR}
)

# Install header files.
file(GLOB headerFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
set(headers_install_dir ${DEVELOP_DIR}/headers/os)
install(FILES ${headerFiles} DESTINATION ${headers_install_dir})
