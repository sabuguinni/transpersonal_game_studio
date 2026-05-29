#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "DestructionPhysicsComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode"),
    Slice           UMETA(DisplayName = "Slice")
};

UENUM(BlueprintType)
enum class ECore_DestructionState : uint8
{
    Intact          UMETA(DisplayName = "Intact"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Destroyed       UMETA(DisplayName = "Destroyed"),
    Cleanup         UMETA(DisplayName = "Cleanup")
};

USTRUCT(BlueprintType)
struct FCore_DestructionFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragment")
    UStaticMesh* FragmentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragment")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragment")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragment")
    float LifeTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fragment")
    bool bSimulatePhysics = true;

    FCore_DestructionFragment()
    {
        FragmentMesh = nullptr;
        Scale = FVector(1.0f);
        Mass = 1.0f;
        LifeTime = 10.0f;
        bSimulatePhysics = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxFragments = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentMinSize = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentMaxSize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ExplosionForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifeTime = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bGenerateDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCreateDust = true;

    FCore_DestructionSettings()
    {
        DestructionType = ECore_DestructionType::Fracture;
        DestructionThreshold = 100.0f;
        MaxFragments = 20;
        FragmentMinSize = 0.1f;
        FragmentMaxSize = 1.0f;
        ExplosionForce = 500.0f;
        FragmentLifeTime = 15.0f;
        bGenerateDebris = true;
        bCreateDust = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_DestructionImpact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    AActor* ImpactCauser = nullptr;

    FCore_DestructionImpact()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactDirection = FVector::ZeroVector;
        ImpactForce = 0.0f;
        ImpactCauser = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionTriggered, ECore_DestructionType, DestructionType, FCore_DestructionImpact, Impact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestructionStateChanged, ECore_DestructionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFragmentSpawned, AActor*, Fragment);

UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDestructionPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDestructionPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Destruction Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FCore_DestructionImpact& Impact);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestructionAtLocation(const FVector& Location, float Force, AActor* Causer = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionSettings(const FCore_DestructionSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void AddDamage(float DamageAmount, const FVector& DamageLocation = FVector::ZeroVector);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Destruction")
    ECore_DestructionState GetDestructionState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool IsDestroyed() const { return CurrentState == ECore_DestructionState::Destroyed; }

    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetDamageAccumulated() const { return AccumulatedDamage; }

    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetDamagePercentage() const;

    // Fragment Management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void AddCustomFragment(const FCore_DestructionFragment& Fragment);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ClearFragments();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupDebris();

    // Advanced Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void PreviewDestruction();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void RestoreFromDestruction();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Destruction")
    void GenerateFragments();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDestructionTriggered OnDestructionTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDestructionStateChanged OnDestructionStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFragmentSpawned OnFragmentSpawned;

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionSettings DestructionSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fragments", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_DestructionFragment> CustomFragments;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ECore_DestructionState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float AccumulatedDamage;

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UParticleSystem* DestructionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    USoundCue* DestructionSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
    UMaterialInterface* DamagedMaterial;

    // Component References
    UPROPERTY()
    UStaticMeshComponent* StaticMeshComponent;

    UPROPERTY()
    UInstancedStaticMeshComponent* FragmentInstancedMesh;

    // Runtime Data
    UPROPERTY()
    TArray<AActor*> SpawnedFragments;

    FTimerHandle CleanupTimerHandle;
    UStaticMesh* OriginalMesh;
    UMaterialInterface* OriginalMaterial;

private:
    // Internal Functions
    void ExecuteDestruction(const FCore_DestructionImpact& Impact);
    void SpawnFragments(const FCore_DestructionImpact& Impact);
    void SpawnSingleFragment(const FCore_DestructionFragment& Fragment, const FVector& SpawnLocation, const FVector& Velocity);
    void CreateDestructionEffects(const FVector& Location);
    void UpdateDestructionState();
    void ScheduleCleanup();
    void PerformCleanup();
    
    // Fragment Generation
    void GenerateProceduralFragments();
    FCore_DestructionFragment CreateRandomFragment(const FVector& BaseSize);
    UStaticMesh* CreateFragmentMesh(const FVector& Size);
    
    // Damage System
    void ApplyDamageVisuals();
    bool ShouldTriggerDestruction() const;
};