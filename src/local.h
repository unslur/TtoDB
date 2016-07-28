#define BUF_SIZE 1024                 //Ĭ�ϻ�����
//#define serverport 15555             //����˿�
//#define serverip "192.168.9.227"   //������IP��ַ
#define EPOLL_RUN_TIMEOUT -1          //epoll�ĳ�ʱʱ��
#define EPOLL_SIZE 1000              //epoll����Ŀͻ��˵������Ŀ

#define STR_WELCOME "Welcome to seChat! You ID is: Client #%d"
#define STR_MESSAGE "Client #%d>> %s"
#define STR_NOONE_CONNECTED "Noone connected to server except you!"
#define CMD_EXIT "EXIT"

//�������õĺ궨�壺���͸�ֵ���Ҽ��
#define CHK(eval) if(eval < 0){perror("eval"); return 0;}
#define CHK2(res, eval) if((res = eval) < 0){perror("eval"); return 0;}

//================================================================================================
//������                  setnonblocking
//����������                ����socketΪ������
//���룺                    [in] sockfd socket��ʾ��
//�����                    ��
//���أ�                    0
//================================================================================================
int setnonblocking(int sockfd);

//================================================================================================
//������                  handle_message
//����������                ����ÿ���ͻ���socket
//���룺                    [in] new_fd socket��ʾ��
//�����                    ��
//���أ�                    ���شӿͻ��˽��ܵ���ݵĳ���
//================================================================================================
int handle_message(int new_fd);
