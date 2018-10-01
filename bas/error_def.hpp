#ifndef __ERROR_DEF_HPP_2015_07_24__
#define __ERROR_DEF_HPP_2015_07_24__
/*
*	�����붨�����
*	��������32λ�޷������α�ʾ���������
*
*	|	ģ��	  |		���������	 |
*	|__8bits__|______24bits______|
*
*	���к���ִ�гɹ�����0�����򷵻���Ӧ������
*
*	ʹ��ʾ��
*
*	ʹ��ʾ��1��Ԥ�ȶ�����������ƣ���
*
*	SET_MODULE_ERR_BAS(module_sock, 1)
*	enum socket_error
*	{
*		E_CREATE_ERROR	= 0x01,
*		E_INVALID_SOCK	= 0x02,
*		E_PEER_CLOSE	= 0x03
*	};
*
*	BEGIN_ERROR_CODE(SOCK)
*	DEFINE_ERROR_CODE(CREATE_ERROR, module_sock, E_CREATE_ERROR);
*	DEFINE_ERROR_CODE(INVALID_SOCK, module_sock, E_INVALID_SOCK);
*	DEFINE_ERROR_CODE(PEER_CLOSE,	module_sock, E_PEER_CLOSE);
*	END_ERROR_CODE()
*
*	unsigned int fun()
*	{
*		...
*		if(some error)
*			return ERROR_CODE(SOCK, INVALID_SOCK);
*		else
*			return ERROR_CODE(SOCK, NO_ERR);
*	}
*
*	ʹ��ʾ��2����������������ƣ���
*
*	SET_MODULE_ERR_BAS(module_sock, 1)
*	unsigned int fun()
*	{
*		int err = some_3rd_fun();	//	some_3rd_fun����-1����ɹ�
*		if(err == -1) err = 0;
*		return GENERATE_ERROR(module_sock, err);
*	}
*/

#define MOD_0 0

#define GENERATE_ERROR(module, code) \
	(code == 0) ? 0 : (module << 24) | code

#define DEFINE_ERROR_CODE(err_name, module, code) \
	static const unsigned int ERR_##err_name = \
	GENERATE_ERROR(MOD_##module, code)

#define ERROR_CODE(module_name, err_name) \
	module_name::ERR_##err_name

#define BEGIN_ERROR_CODE(module_name) \
	namespace module_name \
	{ \
		DEFINE_ERROR_CODE(NO_ERR, 0, 0);

#define END_ERROR_CODE() \
	}

#define SET_MODULE_ERR_BAS(module, num) \
	static const unsigned int MOD_##module = num;

#endif
