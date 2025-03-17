cmake_minimum_required(VERSION 3.16)

qt_standard_project_setup()
add_subdirectory(lib)
qt_add_executable(${PROJECT_NAME}
        main.cpp
)
qt6_add_resources(${PROJECT_NAME}
        PREFIX "/"
        FILES
        qtquickcontrols2.conf
)
target_include_directories(${PROJECT_NAME}
        PRIVATE
        model/annotation
        model/category
        model/filesystem
        model/history
        common/project
        common/signalBus
)
qt_add_qml_module(${PROJECT_NAME}
        URI ${PROJECT_NAME}
        RESOURCE_PREFIX "/buf.me/imports"
        VERSION 1.0
        QML_FILES
        view/Main.qml
        view/AnnotationArea.qml
        view/SideBar.qml
        view/ImageViewer.qml
        view/components/AnnotationCanvas.qml
        view/components/CategoryLegend.qml
        view/components/CategorySelector.qml
        view/components/menus/FileMenu.qml
        view/components/menus/CategoryMenu.qml
        view/components/menus/HelpMenu.qml
        view/components/dialogs/FolderSelectionDialog.qml
        view/components/dialogs/MessageDialog.qml
        view/components/dialogs/ImportCategoriesDialog.qml
        view/components/dialogs/ExportCategoriesDialog.qml
        view/components/dialogs/SaveDialog.qml
        view/components/dialogs/ImageFileDialog.qml
        view/components/dialogs/ShortcutsDialog.qml
        SOURCES
        model/annotation/AnnotationManager.h
        model/annotation/AnnotationManager.cpp
        model/filesystem/FileSystemModel.h
        model/filesystem/FileSystemModel.cpp
        model/category/CategoryManager.h
        model/category/CategoryManager.cpp
        model/history/OperationHistoryManager.h
        model/history/OperationHistoryManager.cpp
        common/signalBus/SignalBus.h
        common/signalBus/SignalBus.cpp
        common/project/ProjectManager.cpp
        common/project/ProjectManager.h
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt6::Quick
        OGPRParser
        RadarProcessor
)

