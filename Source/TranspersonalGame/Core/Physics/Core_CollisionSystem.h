#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "Core_CollisionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    Player,
    Dinosaur,
    Environment,
    Projectile,
    Trigger
};

USTRUCT(BlueprintType)
struct FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    ECore_CollisionType CollisionType = ECore_CollisionType::Environment;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    float Damage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    FVector ImpactForce = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    bool bCanBlock = true;

    FCore_CollisionData()
    {
        CollisionType = ECore_CollisionType::Environment;
        Damage = 0.0f;
        ImpactForce = FVector::ZeroVector;
        bCanBlock = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionType CollisionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ImpactForceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableCollisionEvents;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionType(ECore_CollisionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    ECore_CollisionType GetCollisionType() const;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ProcessCollision(const FHitResult& HitResult, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    FCore_CollisionData GetCollisionData() const;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ApplyImpactForce(AActor* TargetActor, const FVector& ImpactPoint, float Force);

    UFUNCTION(BlueprintImplementableEvent, Category = "Collision")
    void OnCollisionProcessed(const FCore_CollisionData& CollisionData, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void EnableCollisionEvents(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool IsCollisionEnabled() const;

private:
    void HandleDinosaurCollision(AActor* DinosaurActor, const FHitResult& HitResult);
    void HandlePlayerCollision(AActor* PlayerActor, const FHitResult& HitResult);
    void HandleEnvironmentCollision(AActor* EnvironmentActor, const FHitResult& HitResult);
    void CalculateImpactForce(const FHitResult& HitResult, FVector& OutForce);
};