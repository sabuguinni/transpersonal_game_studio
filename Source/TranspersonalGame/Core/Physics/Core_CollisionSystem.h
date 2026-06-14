#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "SharedTypes.h"
#include "Core_CollisionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    None UMETA(DisplayName = "None"),
    Static UMETA(DisplayName = "Static"),
    Dynamic UMETA(DisplayName = "Dynamic"),
    Character UMETA(DisplayName = "Character"),
    Projectile UMETA(DisplayName = "Projectile"),
    Trigger UMETA(DisplayName = "Trigger")
};

USTRUCT(BlueprintType)
struct FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionType CollisionType = ECore_CollisionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanBlock = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanOverlap = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Collision Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    FCore_CollisionData CollisionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    TArray<TEnumAsByte<EObjectTypeQuery>> CollisionObjectTypes;

    // Collision Detection Methods
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    TArray<FHitResult> MultiLineTrace(const FVector& Start, const FVector& End);

    // Overlap Detection
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool SphereOverlap(const FVector& Location, float Radius, TArray<AActor*>& OverlappingActors);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool BoxOverlap(const FVector& Location, const FVector& HalfSize, TArray<AActor*>& OverlappingActors);

    // Collision Response
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetCollisionEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetCollisionType(ECore_CollisionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void UpdateCollisionProfile();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Collision System")
    void OnCollisionDetected(const FHitResult& HitResult);

    UFUNCTION(BlueprintImplementableEvent, Category = "Collision System")
    void OnOverlapBegin(AActor* OtherActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Collision System")
    void OnOverlapEnd(AActor* OtherActor);

private:
    // Internal collision handling
    UFUNCTION()
    void HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void HandleActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

    // Cached references
    UPROPERTY()
    UPrimitiveComponent* PrimaryCollisionComponent;

    // Collision tracking
    UPROPERTY()
    TArray<AActor*> CurrentOverlaps;

    bool bCollisionEnabled = true;
    float LastCollisionTime = 0.0f;
};