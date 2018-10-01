#include <bas/memory.hpp>
#include <json/json.hpp>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "basd.lib")
BIO_AUTOLOAD("json");

const char json_str[] = "{\"Addr\":	\"192.168.20.11\", \"Port\": 11, \
	\"ServiceList\": \
	[ \
		{ \
			\"SubAddr0\": \"192.168.20.189\", \
			\"SubPort0\": 10085 \
		}, \
		{ \
			\"SubAddr1\": \"192.168.20.190\", \
			\"SubPort1\": 10086 \
		} \
	]}";

void main()
{
	{	//	编码
		CJSon json;
		int32 port_resv[] = { 10085, 10086, 10087, 10088 };

		json.new_root().attach_object()
			.set_string("Addr", "192.168.20.11").set_int32("Port", 11)
			.new_array("ServiceList").attach_object()
			.set_string("SubAddr1", "192.168.20.189").set_int_group("SubPort1", port_resv, sizeof(port_resv) / sizeof(int32))
			.to_upper_level().attach_object()
			.set_string("SubAddr2", "192.168.20.190").set_int_group("SubPort2", port_resv, sizeof(port_resv) / sizeof(int32));

		pchar str = json.encode();
		json.json_mem_free((pointer)str);
	}

	{	//	解码
		CJSon json;

		json.decode_from_stream(json_str);

		VALUE addr_v = json.get_item_from_name("Addr").get_item_value();	//	改变 json 对象状态，get_root 可在任何层次直接获取根节点
		json.get_root();

		VALUE port_v = json.get_item_from_name("Port").get_item_value();
		json.get_root();

		printf("Addr : %s, Port : %d\n", addr_v.str_val, port_v.iv.int_val);

		cpchar name = json.get_sub_item().get_next_item().get_next_item().get_item_name();	//	json.get_item_from_name("ServiceList");
		printf("%s\n", name);

		CJSon head = json;

		if(json.get_item_type() == IT_ARRAY)
		{
			for(int32 i = 0; i < json.get_array_size(); i++)
			{
				char addr_name[20] = {};
				char port_name[20] = {};
				sprintf(addr_name, "SubAddr%d", i);
				sprintf(port_name, "SubPort%d", i);

				VALUE addr_v = json.get_array_item_from_pos(i).get_item_from_name(addr_name).get_item_value();	//	改变 json 对象状态，用保存的 head 恢复
				json = head;
				VALUE port_v = json.get_array_item_from_pos(i).get_item_from_name(port_name).get_item_value();
				json = head;
				printf("Addr : %s, Port : %d\n", addr_v.str_val, port_v.iv.int_val);
			}
		}
	}

	getchar();
}
