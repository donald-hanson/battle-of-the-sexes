#ifndef NETMSG_H
#define NETMSG_H

int NET_ReadByte();
void NET_WriteByte(int c);

int NET_ReadLong();
void NET_WriteLong(int c);

int NET_ReadBits(int bits);
void NET_WriteBits(int value, int bits);

float NET_ReadFloat();
void NET_WriteFloat(float value);

#endif