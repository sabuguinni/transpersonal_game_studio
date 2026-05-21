#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core_DestructionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float BreakThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactDamageMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxFragments = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bEnableDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DebrisScale = 0.1f;

    FCore_DestructionProfile()
    {
        BreakThreshold = 1000.0f;
        ImpactDamageMultiplier = 1.5f;
        MaxFragments = 50;
        FragmentLifetime = 30.0f;
        bEnableDebris = true;
        DebrisScale = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Destruction")
    AActor* DestroyedActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction")
    FVector ImpactVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction")
    float DamageAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction")
    int32 FragmentCount = 0;

    FCore_DestructionEvent()
    {
        DestroyedActor = nullptr;
        ImpactLocation = FVector::ZeroVector;
        ImpactVelocity = FVector::ZeroVector;
        DamageAmount = 0.0f;
        FragmentCount = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestructionEvent, const FCore_DestructionEvent&, DestructionData);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core destruction functionality
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void InitializeDestructionSystem();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool ProcessImpactDestruction(AActor* TargetActor, const FVector& ImpactLocation, const FVector& ImpactVelocity, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateDestructionFragments(AActor* SourceActor, const FVector& ImpactLocation, int32 FragmentCount);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDestructionForce(const FVector& ForceLocation, float ForceRadius, float ForceStrength);

    // Destruction profiles
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionProfile(const FCore_DestructionProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    FCore_DestructionProfile GetDestructionProfile() const { return DestructionProfile; }

    // Fragment management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupExpiredFragments();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    int32 GetActiveFragmentCount() const { return ActiveFragments.Num(); }

    // Chaos Physics integration
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void EnableChaosDestruction(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ConfigureChaosBreaking(UStaticMeshComponent* MeshComponent, float BreakThreshold);

    // Geometry Collection support
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetupGeometryCollection(AActor* TargetActor, class UGeometryCollection* GeometryCollection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerGeometryCollectionBreak(AActor* TargetActor, const FVector& BreakLocation);

    // Field System integration
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateDestructionField(const FVector& FieldLocation, float FieldRadius, float FieldStrength);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Destruction", CallInEditor)
    void DebugDrawDestructionInfo();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ToggleDestructionDebug() { bShowDebugInfo = !bShowDebugInfo; }

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void GetDestructionStats(int32& TotalDestructions, int32& ActiveFragments, float& AverageProcessTime);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDestructionEvent OnDestructionEvent;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionProfile DestructionProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bEnableDestruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bUseChaosPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bUseGeometryCollection = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    float MaxDestructionDistance = 10000.0f;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> ActiveFragments;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_DestructionEvent> RecentDestructions;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    bool bShowDebugInfo = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    bool bDrawDestructionSphere = true;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 TotalDestructionCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AverageDestructionProcessTime = 0.0f;

private:
    // Internal helpers
    bool CanDestroyActor(AActor* Actor) const;
    void CreatePhysicsFragments(AActor* SourceActor, const FVector& ImpactLocation, int32 FragmentCount);
    void ApplyFragmentPhysics(AActor* Fragment, const FVector& ImpactVelocity);
    void RegisterDestructionEvent(const FCore_DestructionEvent& Event);
    void UpdatePerformanceStats(float ProcessTime);

    // Timers
    FTimerHandle FragmentCleanupTimer;
};

#include "Core_DestructionSystem.generated.h"