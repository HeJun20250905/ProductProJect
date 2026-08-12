#pragma once

#ifdef LIBDATABASE_EXPORTS
#define LIBDATABASE_API __declspec(dllexport)
#else
#define LIBDATABASE_API __declspec(dllimport)
#endif

// 返回状态码
typedef enum LIBDATABASE_RESULT_STATUS
{
    LIBDATABASE_RESULT_OK = 0,							            // 成功
    LIBDATABASE_RESULT_ERR_NOT_INIT = -1,							// 未初始化或已卸载
    LIBDATABASE_RESULT_ERR_QT_ENV = -2,							    // Qt环境创建/获取失败
    LIBDATABASE_RESULT_ERR_TRANS = -3,							    // 翻译器加载失败
    LIBDATABASE_RESULT_ERR_INVALID_PARAM = -4,						// 无效参数
    LIBDATABASE_RESULT_ERR_UNKNOWN = -99							// 未知错误
} LIBDATABASE_RESULT_STATUS;

typedef struct LibraryItem
{
	// 核心标识（必须有）
	char name[128]; 				// 元件型号 (如 "0402_10K", "SOP8")，作为全局元件库 QHash 的 Key

	// 物理尺寸
	float length;					// 元件长度 (mm)
	float width;					// 元件宽度 (mm)
	float height;					// 元件高度/厚度 (mm)

	// 贴装工艺参数
	float pickZ;					// 取料高度 (mm)：吸嘴下探到料带/托盘吸取的深度
	float placeZ;					// 贴装高度 (mm)：吸嘴下压到 PCB 板上的贴装深度
	int nozzleId;					// 吸嘴编号 (如 1, 2, 3)：告诉机器换哪个吸嘴来吸取这个元件
	float speed;					// 贴装速度：不同大小、重量的元件，下压的速度要求不同
}LibraryItem;


#ifdef __cplusplus
extern "C"
{
#endif

    //初始化设备引擎
    LIBDATABASE_API int LibDatabase_InitDevice();

    // 增加元件库数据（使用指针传递）
    LIBDATABASE_API int LibDatabase_InsertLibraryItem(const LibraryItem* item);

    // 删除元件库数据
    LIBDATABASE_API int LibDatabase_DeleteLibraryItem(const char* libraryName);

    // 修改元件库数据（使用指针传递）
    LIBDATABASE_API int LibDatabase_UpdateLibraryItem(const LibraryItem* item);

    // 查询元件库数据（使用指针传递）
    LIBDATABASE_API int LibDatabase_QueryLibraryItem(LibraryItem* item, const char* libraryName);
    
    // 初始化数据结构
    LIBDATABASE_API int LibDatabase_OnInitDataStruct(LibraryItem* data);

    // 卸载/清理资源 (安全释放DLL内部内存)
    LIBDATABASE_API int LibDatabase_UninitDevice();

#ifdef __cplusplus
}
#endif
