#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "Kismet/GameplayStatics.h"
#include "Core_CollisionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionProfile : uint8
{
    Default         UMETA(DisplayName = "Default"),
    Character       UMETA(DisplayName = "Character"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Environment     UMETA(DisplayName = "Environment"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Trigger         UMETA(DisplayName = "Trigger"),
    Destructible    UMETA(DisplayName = "Destructible")
};

UENUM(BlueprintType)
enum class ECore_CollisionResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Overlap         UMETA(DisplayName = "Overlap"),
    Block           UMETA(DisplayName = "Block"),
    Custom          UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionProfile CollisionProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bGenerateOverlapEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanCharacterStepUpOn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionHeight;

    FCore_CollisionSettings()
    {
        CollisionProfile = ECore_CollisionProfile::Default;
        bEnableCollision = true;
        bGenerateOverlapEvents = false;
        bCanCharacterStepUpOn = true;
        CollisionRadius = 50.0f;
        CollisionHeight = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Collision Event")
    AActor* HitActor;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Event")
    UPrimitiveComponent* HitComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Event")
    FVector ImpactPoint;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Event")
    FVector ImpactNormal;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Event")
    float ImpactForce;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Event")
    float Timestamp;

    FCore_CollisionEvent()
    {
        HitActor = nullptr;
        HitComponent = nullptr;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::ZeroVector;
        ImpactForce = 0.0f;
        Timestamp = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCore_OnCollisionEvent, const FCore_CollisionEvent&, CollisionEvent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    FCore_CollisionSettings CollisionSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    bool bEnableAdvancedCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    float CollisionCheckFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    int32 MaxCollisionEvents;

    // Collision History
    UPROPERTY(BlueprintReadOnly, Category = "Collision System")
    TArray<FCore_CollisionEvent> RecentCollisions;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FCore_OnCollisionEvent OnCollisionDetected;

    // Internal State
    float LastCollisionCheckTime;
    int32 CollisionEventCount;

public:
    // Configuration Methods
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetCollisionProfile(ECore_CollisionProfile NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetCollisionEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetCollisionSettings(const FCore_CollisionSettings& NewSettings);

    // Query Methods
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool IsCollisionEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    ECore_CollisionProfile GetCurrentCollisionProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    TArray<FCore_CollisionEvent> GetRecentCollisions() const;

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool PerformLineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool PerformSphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    TArray<AActor*> GetOverlappingActors(float SearchRadius = 500.0f);

    // Event Handlers
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Utility Methods
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void ClearCollisionHistory();

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void ApplyCollisionSettings();

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    float GetCollisionIntensity(const FCore_CollisionEvent& Event) const;

private:
    void UpdateCollisionDetection(float DeltaTime);
    void ProcessCollisionEvent(const FHitResult& Hit, float ImpactForce);
    void RegisterCollisionEvent(const FCore_CollisionEvent& Event);
    UPrimitiveComponent* GetOwnerPrimitiveComponent() const;
};