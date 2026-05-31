#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/DestructibleActor.h"
#include "PhysicsEngine/DestructibleComponent.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode")
};

UENUM(BlueprintType)
enum class ECore_DestructionState : uint8
{
    Intact          UMETA(DisplayName = "Intact"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Destroyed       UMETA(DisplayName = "Destroyed"),
    Rebuilding      UMETA(DisplayName = "Rebuilding")
};

USTRUCT(BlueprintType)
struct FCore_DestructionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxFragments = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bAutoCleanup = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    FCore_DestructionSettings()
    {
        DestructionThreshold = 100.0f;
        ImpactMultiplier = 1.0f;
        MaxFragments = 50;
        FragmentLifetime = 10.0f;
        bAutoCleanup = true;
        DestructionType = ECore_DestructionType::Fracture;
    }
};

USTRUCT(BlueprintType)
struct FCore_DestructionFragment
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fragment")
    AActor* FragmentActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fragment")
    FVector InitialVelocity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fragment")
    float CreationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fragment")
    bool bIsActive;

    FCore_DestructionFragment()
    {
        FragmentActor = nullptr;
        InitialVelocity = FVector::ZeroVector;
        CreationTime = 0.0f;
        bIsActive = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Destruction Control Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(float ImpactForce = 0.0f, FVector ImpactLocation = FVector::ZeroVector, FVector ImpactDirection = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, FVector ImpactLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void RestoreObject();

    UFUNCTION(BlueprintPure, Category = "Destruction")
    ECore_DestructionState GetDestructionState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetDamageAccumulated() const { return AccumulatedDamage; }

    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool IsDestroyed() const { return CurrentState == ECore_DestructionState::Destroyed; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionSettings(const FCore_DestructionSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Destruction")
    FCore_DestructionSettings GetDestructionSettings() const { return Settings; }

    // Fragment Management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    UFUNCTION(BlueprintPure, Category = "Destruction")
    TArray<FCore_DestructionFragment> GetActiveFragments() const { return ActiveFragments; }

    UFUNCTION(BlueprintPure, Category = "Destruction")
    int32 GetFragmentCount() const { return ActiveFragments.Num(); }

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestructionStateChanged, ECore_DestructionState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnDestructionStateChanged OnDestructionStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectDestroyed, float, ImpactForce, FVector, ImpactLocation, FVector, ImpactDirection);
    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnObjectDestroyed OnObjectDestroyed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectRestored);
    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnObjectRestored OnObjectRestored;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction")
    ECore_DestructionState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    FCore_DestructionSettings Settings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction")
    float AccumulatedDamage;

    UPROPERTY()
    UStaticMeshComponent* StaticMeshComp;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UStaticMesh* OriginalMesh;

    UPROPERTY()
    USkeletalMesh* OriginalSkeletalMesh;

    UPROPERTY()
    TArray<FCore_DestructionFragment> ActiveFragments;

    UPROPERTY()
    FTransform OriginalTransform;

private:
    void InitializeDestructionSystem();
    void CreateFragments(float ImpactForce, FVector ImpactLocation, FVector ImpactDirection);
    void UpdateFragments(float DeltaTime);
    void SetDestructionState(ECore_DestructionState NewState);
    void HideOriginalMesh();
    void ShowOriginalMesh();
    void BroadcastStateChange(ECore_DestructionState NewState);
    AActor* CreateFragment(FVector Location, FVector Velocity, UStaticMesh* FragmentMesh);
};