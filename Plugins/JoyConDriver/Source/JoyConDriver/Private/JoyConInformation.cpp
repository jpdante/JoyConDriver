// Fill out your copyright notice in the Description page of Project Settings.


#include "JoyConInformation.h"

#include "hidapi.h"

FJoyConInformation::FJoyConInformation() : ProductId(0), VendorId(0), InterfaceNumber(0), ReleaseNumber(0),
ProbableControllerIndex(0),
Usage(0),
UsagePage(0),
IsLeft(false),
IsConnected(false) {}

FJoyConInformation::FJoyConInformation(
	const int TempProductId,
	const int TempVendorId,
	const int TempInterfaceNumber,
	const int TempReleaseNumber,
	const FString TempManufacturerString,
	const FString TempBluetoothPath,
	const FString TempProductString,
	const FString TempSerialNumber,
	const int TempProbableControllerIndex,
	const int TempUsage,
	const int TempUsagePage,
	const bool TempIsLeft,
	const bool TempIsConnected) {
	ProductId = TempProductId;
	VendorId = TempVendorId;
	InterfaceNumber = TempInterfaceNumber;
	ReleaseNumber = TempReleaseNumber;
	ManufacturerString = TempManufacturerString;
	BluetoothPath = TempBluetoothPath;
	ProductString = TempProductString;
	SerialNumber = TempSerialNumber;
	ProbableControllerIndex = TempProbableControllerIndex;
	Usage = TempUsage;
	UsagePage = TempUsagePage;
	IsLeft = TempIsLeft;
	IsConnected = TempIsConnected;
}
