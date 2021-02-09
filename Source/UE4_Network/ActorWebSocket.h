// by Tiax 2020/11/26

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "ActorWebSocket.generated.h"

UCLASS()
class UE4_NETWORK_API AActorWebSocket : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActorWebSocket();

	const FString ServerURL="ws://192.168.100.101:8887";
	const FString ServerProtocol="ws";

	TSharedPtr<IWebSocket> Socket=nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void OnConnected();
	void OnConnectionError(const FString& Error);
	void OnClosed(int32 StatusCode,const FString& Reason,bool bWasClean);
	void OnMessage(const FString& Message); // 接收消息时
	void OnMessageSent(const FString& MessageString); // 发送消息时

	void JsonParse(const FString InMessage);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void MySend();
};
