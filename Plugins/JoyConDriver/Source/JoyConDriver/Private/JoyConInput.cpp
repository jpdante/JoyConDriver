// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyConInput.h"

#include "hidapi.h"
#include "JoyConState.h"
#include "Engine/Engine.h"
#include "HAL/RunnableThread.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "JoyConInput"

// Setup Keys
const FKey FJoyConKey::JoyCon_DPad_Up("JoyCon_DPad_Up");
const FKey FJoyConKey::JoyCon_DPad_Left("JoyCon_DPad_Left");
const FKey FJoyConKey::JoyCon_DPad_Right("JoyCon_DPad_Right");
const FKey FJoyConKey::JoyCon_DPad_Down("JoyCon_DPad_Down");

const FKey FJoyConKey::JoyCon_Minus("JoyCon_Minus");
const FKey FJoyConKey::JoyCon_Plus("JoyCon_Plus");
const FKey FJoyConKey::JoyCon_Home("JoyCon_Home");
const FKey FJoyConKey::JoyCon_Capture("JoyCon_Capture");

const FKey FJoyConKey::JoyCon_Analog_Click("JoyCon_Analog_Click");

const FKey FJoyConKey::JoyCon_Sr("JoyCon_Sr");
const FKey FJoyConKey::JoyCon_Sl("JoyCon_Sl");

const FKey FJoyConKey::JoyCon_Shoulder_1("JoyCon_Shoulder_1");
const FKey FJoyConKey::JoyCon_Shoulder_2("JoyCon_Shoulder_2");

// Setup Keys Names

const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_DPad_Up("JoyCon_DPad_Up");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_DPad_Left("JoyCon_DPad_Left");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_DPad_Right("JoyCon_DPad_Right");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_DPad_Down("JoyCon_DPad_Down");

const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Minus("JoyCon_Minus");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Plus("JoyCon_Plus");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Home("JoyCon_Home");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Capture("JoyCon_Capture");

const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Analog_Click("JoyCon_Analog_Click");

const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Sr("JoyCon_Sr");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Sl("JoyCon_Sl");

const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Shoulder_1("JoyCon_Shoulder_1");
const FJoyConKeyNames::Type FJoyConKeyNames::JoyCon_Shoulder_2("JoyCon_Shoulder_2");

float FJoyConInput::InitialButtonRepeatDelay = 0.2f;
float FJoyConInput::ButtonRepeatDelay = 0.1f;

FJoyConInput::FJoyConInput(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) : MessageHandler(InMessageHandler) {
	IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);
	if (hid_init() == 0) HidInitialized = true;
	else {
		HidInitialized = false;
		UE_LOG(LogTemp, Fatal, TEXT("HIDAPI failed to initialize"));
	}
	UE_LOG(LogTemp, Log, TEXT("JoyConDriver is initialized"));
}

FJoyConInput::~FJoyConInput() {
	IModularFeatures::Get().UnregisterModularFeature(GetModularFeatureName(), this);
	if (hid_exit() == 0) HidInitialized = false;
	else {
		HidInitialized = true;
		UE_LOG(LogTemp, Fatal, TEXT("HIDAPI failed to uninitialize"));
	}
	UE_LOG(LogTemp, Log, TEXT("JoyConDriver is uninitialized"));
}

void FJoyConInput::PreInit() {
	// Load the config, even if we failed to initialize a controller
	LoadConfig();

	// Register the FKeys
	EKeys::AddMenuCategoryDisplayInfo("JoyCon", LOCTEXT("JoyConSubCategory", "JoyCon"), TEXT("GraphEditor.PadEvent_16x"));
	
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_DPad_Up, LOCTEXT("JoyCon_DPad_Up", "JoyCon D-pad Up"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_DPad_Left, LOCTEXT("JoyCon_DPad_Left", "JoyCon D-pad Left"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_DPad_Right, LOCTEXT("JoyCon_DPad_Right", "JoyCon D-pad Right"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_DPad_Down, LOCTEXT("JoyCon_DPad_Down", "JoyCon D-pad Down"), FKeyDetails::GamepadKey, "JoyCon"));

	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Minus, LOCTEXT("JoyCon_Minus", "JoyCon Minus"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Plus, LOCTEXT("JoyCon_Plus", "JoyCon Plus"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Home, LOCTEXT("JoyCon_Home", "JoyCon Home"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Capture, LOCTEXT("JoyCon_Capture", "JoyCon Capture"), FKeyDetails::GamepadKey, "JoyCon"));

	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Analog_Click, LOCTEXT("JoyCon_Analog_Click", "JoyCon Analog Click"), FKeyDetails::GamepadKey, "JoyCon"));

	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Sr, LOCTEXT("JoyCon_Sr", "JoyCon Sr"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Sl, LOCTEXT("JoyCon_Sl", "JoyCon Sl"), FKeyDetails::GamepadKey, "JoyCon"));

	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Shoulder_1, LOCTEXT("JoyCon_Shoulder_1", "JoyCon S1"), FKeyDetails::GamepadKey, "JoyCon"));
	EKeys::AddKey(FKeyDetails(FJoyConKey::JoyCon_Shoulder_2, LOCTEXT("JoyCon_Shoulder_2", "JoyCon S2"), FKeyDetails::GamepadKey, "JoyCon"));
	
	UE_LOG(LogTemp, Log, TEXT("JoyConInput pre-init called"));
}

void FJoyConInput::LoadConfig() {
	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("InitialButtonRepeatDelay"), InitialButtonRepeatDelay, GInputIni);
	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("ButtonRepeatDelay"), ButtonRepeatDelay, GInputIni);
}

TArray<FJoyConInformation>* FJoyConInput::SearchJoyCons() {
	TArray<FJoyConInformation>* Data = new TArray<FJoyConInformation>();
	if (!HidInitialized) return Data;
	hid_device_info* Devices = hid_enumerate(0x57e, 0x0);
	if (Devices == nullptr) {
		hid_free_enumeration(Devices);
		Devices = hid_enumerate(0x057e, 0x0);
		if (Devices == nullptr) {
			hid_free_enumeration(Devices);
			return Data;
		}
	}
	hid_device_info* Device = Devices;
	while (Device != nullptr) {
		if (Device->product_id == 0x2006 || Device->product_id == 0x2007) {
			bool IsLeft = false;
			if (Device->product_id == 0x2006) {
				IsLeft = true;
			} else if (Device->product_id == 0x2007) {
				IsLeft = false;
			}
			int ControllerIndex = -1;
			bool IsConnected = false;
			FString SerialNumber(Device->serial_number);
			FString BluetoothPath(Device->path);
			if(Controllers.Num() > 0) {
				for (FJoyConController* Controller : Controllers) {
					if(Controller->JoyConInformation.SerialNumber.Equals(SerialNumber) && Controller->JoyConInformation.BluetoothPath.Equals(BluetoothPath)) {
						IsConnected = true;
						ControllerIndex = Controllers.IndexOfByKey(Controller);
						break;
					}
				}
			}
			const FJoyConInformation JoyConInformation(
				Device->product_id, 
				Device->vendor_id, 
				Device->interface_number, 
				Device->release_number,
				FString(Device->manufacturer_string),
				FString(Device->path),
				FString(Device->product_string),
				FString(Device->serial_number),
				ControllerIndex,
				Device->usage,
				Device->usage_page,
				IsLeft,
				IsConnected
			);
			Data->Add(JoyConInformation);
		}
		Device = Device->next;
	}
	hid_free_enumeration(Devices);
	return Data;
}

bool FJoyConInput::AttachJoyCon(const FJoyConInformation JoyConInformation, int& ControllerIndex) {
	if (!HidInitialized) return false;
	if (JoyConInformation.IsConnected) return false;
	char* Path = TCHAR_TO_ANSI(*JoyConInformation.BluetoothPath);
	hid_device* Handle = hid_open_path(Path);
	hid_set_nonblocking(Handle, 1);
	FJoyConController* Controller = new FJoyConController(JoyConInformation, Handle, true, true, 0.05f, JoyConInformation.IsLeft);
	Controllers.Add(Controller);
	ControllerIndex = Controllers.IndexOfByKey(Controller);
	Controller->JoyConInformation.ProbableControllerIndex = ControllerIndex;
	uint8 Leds = 0x0;
	Leds |= static_cast<uint8>(0x1 << 0);
	Controller->Attach(Leds);
	return true;
}

bool FJoyConInput::DetachJoyCon(const int ControllerIndex) {
	if (!HidInitialized) return false;
	if (ControllerIndex + 1 > Controllers.Num() || ControllerIndex < 0) return false;
	if (Controllers[ControllerIndex] == nullptr) {
		Controllers.RemoveAt(ControllerIndex);
		return false;
	}
	Controllers[ControllerIndex]->Detach();
	Controllers.RemoveAt(ControllerIndex);
	return true;
}

bool FJoyConInput::GetJoyConAccelerometer(const int ControllerIndex, FVector& Out) {
	if (!HidInitialized) return false;
	Out = FVector::ZeroVector;
	if (ControllerIndex + 1 > Controllers.Num() || ControllerIndex < 0) return false;
	if (Controllers[ControllerIndex] == nullptr) {
		Controllers.RemoveAt(ControllerIndex);
		return false;
	}
	Out = Controllers[ControllerIndex]->GetAccelerometer();
	return true;
}

bool FJoyConInput::GetJoyConGyroscope(const int ControllerIndex, FVector& Out) {
	if (!HidInitialized) return false;
	Out = FVector::ZeroVector;
	if (ControllerIndex + 1 > Controllers.Num() || ControllerIndex < 0) return false;
	if (Controllers[ControllerIndex] == nullptr) {
		Controllers.RemoveAt(ControllerIndex);
		return false;
	}
	Out = Controllers[ControllerIndex]->GetGyroscope();
	return true;
}

bool FJoyConInput::GetJoyConVector(const int ControllerIndex, FRotator& Out) {
	if (!HidInitialized) return false;
	Out = FRotator::ZeroRotator;
	if (ControllerIndex + 1 > Controllers.Num() || ControllerIndex < 0) return false;
	if (Controllers[ControllerIndex] == nullptr) {
		Controllers.RemoveAt(ControllerIndex);
		return false;
	}
	Out = Controllers[ControllerIndex]->GetVector();
	return true;
}

void FJoyConInput::Tick(float DeltaTime) {

}

void FJoyConInput::SendControllerEvents() {
	const double CurrentTime = FPlatformTime::Seconds();
	
	for(FJoyConController* Controller : Controllers) {
		Controller->Update();
		for (int32 ButtonIndex = 0; ButtonIndex < static_cast<int32>(EJoyConControllerButton::TotalButtonCount); ++ButtonIndex) {
			FJoyConButtonState& ButtonState = Controller->ControllerState.Buttons[ButtonIndex];
			check(!ButtonState.Key.IsNone()); // is button's name initialized?

			// Determine if the button is pressed down
			const bool bButtonPressed = Controller->Buttons[ButtonIndex];

			// Update button state
			if (bButtonPressed != ButtonState.bIsPressed) {
				ButtonState.bIsPressed = bButtonPressed;
				if (ButtonState.bIsPressed) {
					MessageHandler->OnControllerButtonPressed(ButtonState.Key, Controller->JoyConInformation.ProbableControllerIndex, false);
					
					// Set the timer for the first repeat
					ButtonState.NextRepeatTime = CurrentTime + FJoyConInput::ButtonRepeatDelay;
				} else {
					MessageHandler->OnControllerButtonReleased(ButtonState.Key, Controller->JoyConInformation.ProbableControllerIndex, false);
				}
			}

			// Apply key repeat, if its time for that
			if (ButtonState.bIsPressed && ButtonState.NextRepeatTime <= CurrentTime) {
				MessageHandler->OnControllerButtonPressed(ButtonState.Key, Controller->JoyConInformation.ProbableControllerIndex, true);

				// Set the timer for the next repeat
				ButtonState.NextRepeatTime = CurrentTime + FJoyConInput::ButtonRepeatDelay;
			}
		}
	}
}

void FJoyConInput::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) {
	MessageHandler = InMessageHandler;
}

bool FJoyConInput::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) {
	return false;
}

void FJoyConInput::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) {
}

void FJoyConInput::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) {
}

FName FJoyConInput::GetMotionControllerDeviceTypeName() const {
	const static FName DefaultName(TEXT("JoyConInputDevice"));
	return DefaultName;
}

bool FJoyConInput::GetControllerOrientationAndPosition(const int32 ControllerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const {
	return false;
}

ETrackingStatus FJoyConInput::GetControllerTrackingStatus(const int32 ControllerIndex, const EControllerHand DeviceHand) const {
	return ETrackingStatus::NotTracked;
}

void FJoyConInput::SetHapticFeedbackValues(int32 ControllerId, int32 Hand, const FHapticFeedbackValues& Values) {
}

void FJoyConInput::GetHapticFrequencyRange(float& MinFrequency, float& MaxFrequency) const {
	MinFrequency = 0.f;
	MaxFrequency = 1.f;
}

float FJoyConInput::GetHapticAmplitudeScale() const {
	return 1.f;
}