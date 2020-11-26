// by Tiax 2020/11/26

#include "ActorWebSocket.h"

// Sets default values
AActorWebSocket::AActorWebSocket()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AActorWebSocket::BeginPlay()
{
	Super::BeginPlay();
	
	FModuleManager::Get().LoadModuleChecked("WebSockets");

	Socket=FWebSocketsModule::Get().CreateWebSocket(ServerURL,ServerProtocol);

	// Bind Events
	// Socket->OnConnectionError().AddLambda([](const FString& Error)->
	//         void{UE_LOG(LogTemp,Warning,TEXT("%s"),*Error)}); // Lambda不好看，改用绑定方法
	Socket->OnConnected().AddUObject(this,&AActorWebSocket::OnConnected);
	Socket->OnConnectionError().AddUObject(this,&AActorWebSocket::OnConnectionError);
	Socket->OnClosed().AddUObject(this,&AActorWebSocket::OnClosed);
	Socket->OnMessage().AddUObject(this,&AActorWebSocket::OnMessage);
	Socket->OnMessageSent().AddUObject(this,&AActorWebSocket::OnMessageSent);

	Socket->Connect();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle,this,&AActorWebSocket::MySend,1,true,1);
}

void AActorWebSocket::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Socket->Close();
}

// Called every frame
void AActorWebSocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AActorWebSocket::OnConnected()
{
	UE_LOG(LogTemp,Warning,TEXT("%s"),*FString(__FUNCTION__));
}

void AActorWebSocket::OnConnectionError(const FString& Error)
{
	UE_LOG(LogTemp,Warning,TEXT("%s Error:%s"),*FString(__FUNCTION__),*Error);
}

void AActorWebSocket::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp,Warning,TEXT("%s StatusCode:%d Reason:%s bWasClean:%d"),
		*FString(__FUNCTION__),StatusCode,*Reason,bWasClean);
}

void AActorWebSocket::OnMessage(const FString& Message)
{
	UE_LOG(LogTemp,Warning,TEXT("%s Message:%s"),*FString(__FUNCTION__),*Message);

	JsonParse(Message);
}

void AActorWebSocket::OnMessageSent(const FString& MessageString)
{
	UE_LOG(LogTemp,Warning,TEXT("%s MessageString:%s"),*FString(__FUNCTION__),*MessageString);
}

void AActorWebSocket::MySend()
{
	TSharedPtr<FJsonObject> RootObj=MakeShareable(new FJsonObject);
	RootObj->SetBoolField("IsExecute",true);
	RootObj->SetStringField("Type","Test");
	TSharedPtr<FJsonValue> Value=MakeShareable(new FJsonValueNumber(GetGameTimeSinceCreation()));
	RootObj->SetField("Value",Value);

	FString JsonStr;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter=TJsonWriterFactory<TCHAR>::Create(&JsonStr);
	FJsonSerializer::Serialize(RootObj.ToSharedRef(),JsonWriter);
	
	if(Socket->IsConnected())
	{
		// Socket->Send(FString::SanitizeFloat(GetGameTimeSinceCreation()));
		Socket->Send(JsonStr);
	}

	UE_LOG(LogTemp,Warning,TEXT("%s Result:%s"),*FString(__FUNCTION__),*JsonStr);
}

void AActorWebSocket::JsonParse(const FString InMessage)
{
	TSharedPtr<FJsonObject> RootObj=MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader=TJsonReaderFactory<>::Create(InMessage);

	if(FJsonSerializer::Deserialize(JsonReader,RootObj)) // 通过JsonReader读取RootObj
	{
		FString Topic=RootObj->GetStringField("Topic"); // Topic
		UE_LOG(LogTemp,Warning,TEXT("%s Topic:%s"),*FString(__FUNCTION__),*Topic);
		
		TSharedPtr<FJsonObject> DataObj=MakeShareable(new FJsonObject());
		DataObj=RootObj->GetObjectField("Data"); // Data

		FString Key=DataObj->GetStringField("Key"); // Key
		UE_LOG(LogTemp,Warning,TEXT("%s Key:%s"),*FString(__FUNCTION__),*Key);

		if(Key.Equals("Time"))
		{
			FString Time=DataObj->GetStringField("Value"); // Value type is String
			UE_LOG(LogTemp,Warning,TEXT("%s Time:%s"),*FString(__FUNCTION__),*Time);
		}
		else if(Key.Equals("Something"))
		{
			TArray<TSharedPtr<FJsonValue>> Value=DataObj->GetArrayField("Value"); // Value type is Array

			bool BoolValue=Value[0]->AsBool();
			int IntValue=Value[1]->AsNumber();
			float FloatValue=Value[2]->AsNumber();
			FString StringValue=Value[3]->AsString();

			UE_LOG(LogTemp,Warning,TEXT("%s BoolValue:%d"),*FString(__FUNCTION__),BoolValue);
			UE_LOG(LogTemp,Warning,TEXT("%s IntValue:%d"),*FString(__FUNCTION__),IntValue);
			UE_LOG(LogTemp,Warning,TEXT("%s FloatValue:%f"),*FString(__FUNCTION__),FloatValue);
			UE_LOG(LogTemp,Warning,TEXT("%s StringValue:%s"),*FString(__FUNCTION__),*StringValue);
		}
	}
}