#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Quest_ResourcePickup.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Fiber       UMETA(DisplayName = "Fiber"),
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bFloatAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float FloatAmplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float FloatSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bRotateAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float RotationSpeed;

private:
    UPROPERTY()
    float FloatTimer;

    UPROPERTY()
    FVector InitialLocation;

    UPROPERTY()
    FRotator InitialRotation;

public:
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void InitializeResource(EQuest_ResourceType Type, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetResourceDisplayName() const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SetupVisualEffects();

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    bool CanPlayerPickup(AActor* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void PickupResource(AActor* Player);

    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                       bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                     UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

protected:
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFloatAnimation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateRotationAnimation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetupMeshForResourceType();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawPickupRange();
};