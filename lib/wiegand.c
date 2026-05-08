int WG_Send26(unsigned char *str)
{
	unsigned char one_num	= 0;
	unsigned char even 		= 0;
	unsigned char odd 		= 0;
	unsigned char check_temp,i;

	if(NULL == str)
		return -1;

	/*首先计算2-13位共12位的奇偶*/
	check_temp = *str;
	for(i = 0;i < 8;i++)
	{
		if(check_temp & 0x01)
			one_num++;
		check_temp >>= 1;
	}
	
	check_temp = *(str + 1);
	for(i = 0;i < 4;i++)
	{
		if(check_temp & 0x80)
			one_num++;
		check_temp <<= 1;
	}
	if(one_num % 2 )
		even = 0;
	else
		even = 1;

	
	/*然后计算14-25位共12位的奇偶*/
	one_num = 0;
	check_temp = *(str + 1);
	for(i = 0;i < 4;i++)
	{
		if(check_temp & 0x01)
			one_num++;
		check_temp >>= 1;
	}
	check_temp = *(str + 2);
	for(i = 0;i < 8;i++)
	{
		if(check_temp & 0x01)
			one_num++;
		check_temp >>= 1;
	}
	if(one_num % 2 )
		odd = 1;
	else
		odd = 0;

	/*保持高电平准备发送数据*/
	WG_DATA0(1);
	WG_DATA1(1);
	myDelay_us(5000);

	/*发送第一位校验位*/
	if(even)
	{
		WG_DATA1(0);					
		myDelay_us(300);
		WG_DATA1(1);
	}
	else
	{			
		WG_DATA0(0);				   
		myDelay_us(300);
		WG_DATA0(1);
	}
	myDelay_us(2000);

	/*发送24位数据*/
	for(i = 0;i < 24;i++)
	{
		WG_DATA0(1);
		WG_DATA1(1);
		if(str[0] & 0x80)
		{
			WG_DATA1(0);
			myDelay_us(300);
			WG_DATA1(1);
		}
		else
		{
			WG_DATA0(0);
			myDelay_us(300);
			WG_DATA0(1);
		}
		(*(long*)&str[0]) <<= 1;
		myDelay_us(2);			   
	}
	WG_DATA0(1); //拉高两条数据线电平
	WG_DATA1(1);

	/*发送最后一位校验位*/
	if(odd)
	{
		WG_DATA1(0);
		myDelay_us(300);
		WG_DATA1(1);
	}
	else
	{			
		WG_DATA0(0);
		myDelay_us(300);
		WG_DATA0(1);
	}
	WG_DATA0(1); //拉高两条数据线电平
	WG_DATA1(1);

	return 0;
}

int WG_Send34(unsigned char *str)
{
	unsigned char one_num 	= 0;
	unsigned char even 		= 0;
	unsigned char odd 		= 0;
	unsigned char check_temp,i;

	if(NULL == str)
		return -1;
	
	check_temp = *str; //第一个字节
	for(i = 0;i < 8;i++)
	{
		if(check_temp & 0x01)
			one_num++;
		check_temp >>= 1;
	}
	
	check_temp = *(str + 1);//第二个字节
	for(i = 0;i < 8;i++)
	{
		if(check_temp & 0x01)
			one_num++;
		check_temp >>= 1;
	}
	
	if(one_num % 2 )
		even = 0;
	else
		even = 1;
	
	one_num = 0;
	check_temp = *(str + 2);//第三个字节
	for(i = 0;i < 8;i++)
	{
		if(check_temp & 0x01)
			one_num++;
		check_temp >>= 1;
	}
	check_temp = *(str + 3);//第三个字节
	for(i = 0;i < 8;i++)
	{
		if(check_temp & 0x01)
			one_num++;
		check_temp >>= 1;
	}
	if(one_num % 2 )
		odd = 1;
	else
		odd = 0;
	
	WG_DATA0(1); //拉高两条数据线电平
	WG_DATA1(1);
	myDelay_us(5000);

	/*发送第一位*/
	if(even)
	{
		WG_DATA1(0);
		myDelay_us(300);
		WG_DATA1(1);
	}
	else
	{
		WG_DATA0(0);
		myDelay_us(300);
		WG_DATA0(1);
	}
	myDelay_us(2000);

	/*发送32字节数据*/
	for(i = 0;i < 32;i++)
	{
		WG_DATA0(1);
		WG_DATA1(1);
		if(str[0] & 0x80)
		{
			WG_DATA1(0);
			myDelay_us(300);
			WG_DATA1(1);
		}
		else
		{
			WG_DATA0(0);
			myDelay_us(300);
			WG_DATA0(1);
		}
		(*(long*)&str[0]) <<= 1;
		myDelay_us(2000);
	}
	WG_DATA0(1); //拉高两条数据线电平
	WG_DATA1(1);

	/*发送最后一位校验位*/
	if(odd)
	{
		WG_DATA1(0);
		myDelay_us(300);
		WG_DATA1(1);
	}
	else
	{
		WG_DATA0(0);
		myDelay_us(300);
		WG_DATA0(1);
	}
	WG_DATA0(1); //拉高两条数据线电平
	WG_DATA1(1);

	return 0;
}