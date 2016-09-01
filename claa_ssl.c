#include "claa_ssl.h"

static void claa_ssl_show_certs(SSL *ssl);

uint8 claa_ssl_init(SSL_CTX **ctx)
{
	SSL_CTX *tmpctx = NULL;
	
	SSL_library_init();									/* SSL ���ʼ��*/       
	OpenSSL_add_all_algorithms();						/* ��������SSL �㷨*/  
	SSL_load_error_strings();							/* ��������SSL ������Ϣ*/ 
//	tmpctx = SSL_CTX_new(SSLv3_client_method());  		/* ��SSL V3 ��׼���ݷ�ʽ����һ��SSL_CTX ����SSL Content Text */
	tmpctx = SSL_CTX_new(TLSv1_1_client_method());  	/* ��TLS V1.1 ��׼��ʽ����һ��SSL_CTX ����SSL Content Text */
	if (tmpctx == NULL) 
	{          
		return 0;     
	}

	*ctx = tmpctx;
	
	return 1;
}

void claa_ssl_close(SSL **ssl, SSL_CTX **ctx)
{
	if (!(*ssl))
	{
		SSL_shutdown(*ssl);
		SSL_free(*ssl);
		*ssl = NULL;
	}
	if (!(*ctx))
	{
		SSL_CTX_free(*ctx);
		*ctx = NULL;
	}
}

uint8 claa_ssl_connect(SSL **ssl, SSL_CTX *ctx, sint32 *sockfd)
{
	/* ����ctx ����һ���µ�SSL */     
	*ssl = SSL_new(ctx);     
	SSL_set_fd(*ssl, *sockfd);       
	
	/* ����SSL ����*/      
	if (SSL_connect(*ssl) == -1)
	{
		return 0;     
	}
	else 
	{
		printf("Connected with SSL %s encryption SUCC\n", SSL_get_cipher(*ssl));
		claa_ssl_show_certs(*ssl); 
		return 1;
	}      
}

uint8 claa_ssl_re_connect(SSL **ssl, SSL_CTX **ctx, sint32 *sockfd)
{
	claa_ssl_close(ssl, ctx);

	if (!claa_ssl_init(ctx))
	{
		claa_ssl_close(ssl, ctx);
		return 0;
	}

	if (!sock_reopen(sockfd))
	{
		claa_ssl_close(ssl, ctx);
		return 0;
	}

	if (!claa_ssl_connect(ssl, *ctx, sockfd))
	{
		claa_ssl_close(ssl, ctx);
		sock_close(sockfd);
		return 0;
	}

	return 1;
}

sint32 claa_ssl_send(SSL *ssl, schar *buffer, uint32 len)
{
	uint16 total  = 0;
	uint16 n = 0;
	
	while (len != total)
	{
		/* ���ŷ���len - total���ֽڵ����� */
		n = SSL_write(ssl, buffer+total, len-total);
		if (n <= 0)
		{
			return n;
		}
		/* �ɹ�������n���ֽڵ����� */
		total += n;
	}

	return total;
}

sint32 claa_ssl_recv(SSL *ssl, schar *pbuf, uint32 maxlen)
{
	sint32 datalen=0;
	datalen = SSL_read(ssl, pbuf, maxlen);	
	return datalen;
}

static void claa_ssl_show_certs(SSL *ssl) 
{      
	X509 *cert;     
	char *line;      
	cert = SSL_get_peer_certificate(ssl);     
	
	if (cert != NULL) 
	{          
		printf("����֤����Ϣ:\n");          
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);         
		printf("֤  ��: %s\n", line);         
		free(line);          
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);         
		printf("�䷢��: %s\n", line);         
		free(line);          
		X509_free(cert);     
	} 
	else
	{
		printf("��֤����Ϣ��\n"); 
	}
}

