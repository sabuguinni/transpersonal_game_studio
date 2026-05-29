#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsLayer : uint8
{
    Default = 0,
    Player = 1,
    Dinosaur = 2,
    Environment = 3,
    Projectile = 4,
    Debris = 5,
    Water = 6,
    Vegetation = 7
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Gravity = -980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxPhysicsStepDelta = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableAsyncPhysics = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void InitializePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGravityScale(float NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnableRagdoll(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void DisableRagdoll(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool IsActorInPhysicsLayer(AActor* Actor, ECore_PhysicsLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetActorPhysicsLayer(AActor* Actor, ECore_PhysicsLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    TArray<AActor*> GetActorsInPhysicsLayer(ECore_PhysicsLayer Layer);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    TArray<AActor*> RagdollActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    TMap<ECore_PhysicsLayer, TArray<AActor*>> LayeredActors;

private:
    void UpdatePhysicsLayers();
    void ProcessRagdollActors(float DeltaTime);
    void ValidatePhysicsSettings();
};

#include "Core_PhysicsSystemManager.generated.h"