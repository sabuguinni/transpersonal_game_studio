#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "NPCBehaviorManager.h"
#include "NPC_BaseNPC.generated.h"

class UNPC_BehaviorComponent;
class UNPC_BehaviorManager;

UENUM(BlueprintType)
enum class ENPC_NPCType : uint8
{
    Friendly    UMETA(DisplayName = "Friendly"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Prey        UMETA(DisplayName = "Prey"),
    Predator    UMETA(DisplayName = "Predator")
};

USTRUCT(BlueprintType)
struct FNPC_NPCStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FleeThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AggressionLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SocialRadius = 500.0f;

    FNPC_NPCStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        MovementSpeed = 300.0f;
        DetectionRadius = 1000.0f;
        FleeThreshold = 0.5f;
        AggressionLevel = 0.3f;
        SocialRadius = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_BaseNPC : public ACharacter
{
    GENERATED_BODY()

public:
    ANPC_BaseNPC();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // NPC Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    ENPC_NPCType NPCType = ENPC_NPCType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    FNPC_NPCStats NPCStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    FString NPCName = TEXT("Generic NPC");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    bool bCanInteract = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    bool bCanBeSocial = true;

    // Detection Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    // Behavior Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNPC_BehaviorComponent* BehaviorComponent;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToAlert(FVector AlertLocation, float AlertIntensity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartSocialInteraction(ANPC_BaseNPC* OtherNPC);

    // Health System
    UFUNCTION(BlueprintCallable, Category = "NPC Health")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "NPC Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "NPC Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Health")
    float GetHealthPercentage() const;

    // Movement Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void MoveToLocation(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void MoveAwayFromLocation(FVector DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void StopMovement();

    // Detection Events
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Getters
    UFUNCTION(BlueprintCallable, Category = "NPC Info")
    FString GetNPCName() const { return NPCName; }

    UFUNCTION(BlueprintCallable, Category = "NPC Info")
    ENPC_NPCType GetNPCType() const { return NPCType; }

    UFUNCTION(BlueprintCallable, Category = "NPC Info")
    FNPC_NPCStats GetNPCStats() const { return NPCStats; }

protected:
    // Internal state
    UPROPERTY()
    TArray<AActor*> DetectedActors;

    UPROPERTY()
    TArray<AActor*> InteractableActors;

    UPROPERTY()
    UNPC_BehaviorManager* BehaviorManager;

    // Internal functions
    void InitializeComponents();
    void RegisterWithBehaviorManager();
    void UnregisterFromBehaviorManager();
    void UpdateDetection(float DeltaTime);
    void ProcessDetectedActors();
    bool ShouldReactToActor(AActor* Actor) const;
    void HandlePlayerDetection(APawn* PlayerPawn);
    void HandleNPCDetection(ANPC_BaseNPC* OtherNPC);
};

#include "NPC_BaseNPC.generated.h"