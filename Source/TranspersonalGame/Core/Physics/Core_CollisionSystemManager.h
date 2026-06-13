#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "SharedTypes.h"
#include "Core_CollisionSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionObjectType::Type> ObjectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<TEnumAsByte<ECollisionResponse::Type>> ResponseArray;

    FCore_CollisionProfile()
    {
        ProfileName = NAME_None;
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        ObjectType = ECollisionObjectType::WorldDynamic;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    AActor* ActorA;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    AActor* ActorB;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector ImpactPoint;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector ImpactNormal;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float ImpactForce;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float TimeStamp;

    FCore_CollisionEvent()
    {
        ActorA = nullptr;
        ActorB = nullptr;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 0.0f;
        TimeStamp = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_CollisionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FCore_CollisionProfile CollisionProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableCollisionLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionDamageMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float MinImpactForceForDamage;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionProfile(const FCore_CollisionProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void EnableCollision(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollisionEvent(const FCore_CollisionEvent& Event);

    UFUNCTION(BlueprintImplementableEvent, Category = "Collision")
    void OnCollisionEventReceived(const FCore_CollisionEvent& Event);

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    TArray<FCore_CollisionEvent> RecentCollisionEvents;

    UPROPERTY()
    float LastCollisionTime;
};

UCLASS()
class TRANSPERSONALGAME_API UCore_CollisionSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    static UCore_CollisionSystemManager* GetCollisionSystemManager(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void RegisterCollisionComponent(UCore_CollisionComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void UnregisterCollisionComponent(UCore_CollisionComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void ProcessCollisionEvent(const FCore_CollisionEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    TArray<FCore_CollisionEvent> GetRecentCollisionEvents(float TimeWindow = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetGlobalCollisionEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void ClearCollisionHistory();

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    int32 GetActiveCollisionComponentCount() const;

protected:
    UPROPERTY()
    TArray<UCore_CollisionComponent*> RegisteredComponents;

    UPROPERTY()
    TArray<FCore_CollisionEvent> GlobalCollisionHistory;

    UPROPERTY()
    bool bGlobalCollisionEnabled;

    UPROPERTY()
    float MaxHistoryTime;

    void CleanupOldEvents();
    void ValidateComponents();
};