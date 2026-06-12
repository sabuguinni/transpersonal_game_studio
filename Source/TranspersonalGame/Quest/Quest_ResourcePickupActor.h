#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Quest_ResourcePickupActor.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Fiber       UMETA(DisplayName = "Fiber"),
    Berries     UMETA(DisplayName = "Berries"),
    Flint       UMETA(DisplayName = "Flint")
};

UCLASS()
class TRANSPERSONALGAME_API AQuest_ResourcePickupActor : public AActor
{
    GENERATED_BODY()
    
public:    
    AQuest_ResourcePickupActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float PickupRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bCanBePickedUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
    bool bShouldRespawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
    float RespawnTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPickedUp;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float RespawnTimer;

public:    
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PickupResource(AActor* PickupActor);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SetResourceType(EQuest_ResourceType NewType);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetResourceTypeString();

    UFUNCTION(BlueprintCallable, Category = "Respawn")
    void StartRespawnTimer();

    UFUNCTION(BlueprintCallable, Category = "Respawn")
    void RespawnResource();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void SetResourceMesh();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void SetVisibility(bool bVisible);
};