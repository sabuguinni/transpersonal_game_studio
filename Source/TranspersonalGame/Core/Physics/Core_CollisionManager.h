#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"
#include "Core_CollisionManager.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    Character UMETA(DisplayName = "Character"),
    Environment UMETA(DisplayName = "Environment"),
    Dinosaur UMETA(DisplayName = "Dinosaur"),
    Projectile UMETA(DisplayName = "Projectile"),
    Interactive UMETA(DisplayName = "Interactive"),
    Trigger UMETA(DisplayName = "Trigger")
};

USTRUCT(BlueprintType)
struct FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionType CollisionType = ECore_CollisionType::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactNormal = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    AActor* HitActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Timestamp = 0.0f;

    FCore_CollisionData()
    {
        CollisionType = ECore_CollisionType::Environment;
        ImpactForce = 0.0f;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::ZeroVector;
        HitActor = nullptr;
        Timestamp = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnCollisionEvent, const FCore_CollisionData&, CollisionData, AActor*, OtherActor);

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FCore_OnCollisionEvent OnCollisionDetected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    ECore_CollisionType MyCollisionType = ECore_CollisionType::Character;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bEnableCollisionLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float MinImpactForceThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float CollisionCooldown = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionHistory = 50;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    TArray<FCore_CollisionData> CollisionHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    int32 TotalCollisions = 0;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollision(const FHitResult& HitResult, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionType(ECore_CollisionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool IsCollisionTypeCompatible(ECore_CollisionType OtherType) const;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void EnableCollisionTracking(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void GetRecentCollisions(TArray<FCore_CollisionData>& OutCollisions, int32 Count = 10);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ClearCollisionHistory();

    UFUNCTION(BlueprintCallable, Category = "Collision")
    float CalculateImpactForce(const FVector& Velocity, float Mass = 1.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool ShouldIgnoreCollision(AActor* OtherActor) const;

private:
    float LastCollisionTime = 0.0f;
    TSet<AActor*> IgnoredActors;
    
    void ProcessCollisionData(const FCore_CollisionData& CollisionData);
    void UpdateCollisionHistory(const FCore_CollisionData& NewCollision);
    bool IsWithinCooldown() const;
};