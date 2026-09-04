@echo off
setlocal

set CMAKE_PATH="C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"

:: pushd 01_BuildEnvironmentSetup
:: pushd 02_VariablesAndControlFlow
:: pushd 03_FunctionsAndScope
:: pushd 04_PointersAndMemory
:: pushd 05_STLContainersAlgorithms
:: pushd 06_ClassesAndOOP
:: pushd 07_ExceptionHandling
:: pushd 08_TemplatesGenericProgramming
:: pushd 11_HelloWindow
:: pushd 12_BasicWidgets
:: pushd 13_EventHandling
:: pushd 14_DialogsAndMessageBox
:: pushd 15_CalculatorApp
:: pushd 16_SimpleTextEditor
:: pushd 17_PaintApp
:: pushd 18_MultiThreadedGUI
:: pushd 21_SQLiteBasicCRUD
:::: pushd 22_MySQLConnection
:: pushd 23_PreparedStatementSecurity
:: pushd 24_TransactionHandling
:: pushd 25_ORMWrapperDesign
:: pushd 26_CustomerManagementApp
:: pushd 27_CSVImportExport

%CMAKE_PATH%\cmake -S . -B build -G "Visual Studio 18 2026" -A x64
popd