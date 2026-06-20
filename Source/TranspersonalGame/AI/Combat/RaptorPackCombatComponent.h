#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RaptorPackCombatComponent.generated.h"

UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Flank       UMETA(DisplayName = "Flank"),
    Lunge       UMETA(DisplayName = "Lunge"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Distract    UMETA(DisplayName = "Distract")
};

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    LeftFlank   UMETA(DisplayName = "LeftFlank"),
    RightFlank  UMETA(DisplayName = "RightFlank")
};

USTRUCT(BlueprintType)
struct FCombat_RaptorPackInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    AActor* AlphaRaptor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float PackAlertLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    bool bPackEngaged = false;
};

USTRUCT(BlueprintType)
struct FCombat_FlankPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FVector WorldPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_RaptorRole AssignedRole = ECombat_RaptorRole::Alpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    bool bPositionReached = false;
};

/**
 * URaptorPackCombatComponent
 *
 * Manages coordinated pack hunting behaviour for Velociraptor groups.
 * Three raptors work together: Alpha distracts, flankers circle and strike.
 * Attach to each Raptor pawn — they communicate via shared pack info.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Pack Combat")
class TRANSPERSONALGAME_API URaptorPackCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorPackCombatComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Configuration ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    ECombat_RaptorRole MyRole = ECombat_RaptorRole::Alpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float LungeRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float LungeDamage = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float LungeCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float FlankRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float StalkSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float LungeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float RetreatSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Config")
    float AlertDecayRate = 5.0f;

    // ── State ──────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor|State")
    ECombat_RaptorState CurrentState = ECombat_RaptorState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor|State")
    FCombat_FlankPosition AssignedFlankPosition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor|State")
    float TimeSinceLastLunge = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor|State")
    bool bFlankPositionReached = false;

    // ── Pack Coordination ──────────────────────────────────────────────────────

    /** Shared pack data — set by Alpha, read by all flankers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Pack")
    FCombat_RaptorPackInfo PackInfo;

    /** Register this raptor into the pack (called by Alpha on BeginPlay) */
    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor|Pack")
    void RegisterWithPack(AActor* AlphaActor);

    /** Alpha broadcasts player location to all pack members */
    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor|Pack")
    void BroadcastPlayerLocation(FVector PlayerLocation);

    /** Compute flanking world position based on role and player location */
    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor|Pack")
    FVector ComputeFlankPosition(FVector PlayerLocation, FVector PlayerForward) const;

    // ── Combat Interface ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ExecuteLunge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void SetState(ECombat_RaptorState NewState);

    // ── Blueprint Events ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Raptor|Events")
    void OnRaptorLunge(AActor* Target, float Damage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Raptor|Events")
    void OnRaptorStateChanged(ECombat_RaptorState OldState, ECombat_RaptorState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Raptor|Events")
    void OnPackEngaged(FVector PlayerLocation);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Raptor|Events")
    void OnRaptorRetreat();

private:
    void TickIdle(float DeltaTime);
    void TickStalk(float DeltaTime);
    void TickFlank(float DeltaTime);
    void TickLunge(float DeltaTime);
    void TickRetreat(float DeltaTime);
    void TickDistract(float DeltaTime);

    bool CanSeePlayer(AActor* Player) const;
    AActor* FindNearestPlayer() const;
    void MoveTowardLocation(FVector TargetLocation, float Speed, float DeltaTime);
    void FaceLocation(FVector TargetLocation, float DeltaTime);

    UPROPERTY()
    AActor* TrackedPlayer = nullptr;

    float DistractTimer = 0.0f;
    float RetreatTimer  = 0.0f;
    float StateTimer    = 0.0f;
};
