#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

// ─── Crowd role enum ─────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECrowd_DinoRole : uint8
{
    HerdMember      UMETA(DisplayName = "Herd Member"),
    PackPredator    UMETA(DisplayName = "Pack Predator"),
    SolitaryWanderer UMETA(DisplayName = "Solitary Wanderer"),
    AlphaLeader     UMETA(DisplayName = "Alpha Leader"),
    Juvenile        UMETA(DisplayName = "Juvenile")
};

// ─── Crowd group struct ───────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCrowd_DinoGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_DinoRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsActive;

    FCrowd_DinoGroup()
        : GroupID(TEXT(""))
        , Role(ECrowd_DinoRole::HerdMember)
        , HomeLocation(FVector::ZeroVector)
        , WanderRadius(2000.f)
        , GroupSize(1)
        , bIsActive(true)
    {}
};

// ─── Manager actor ────────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Crowd groups registered in this manager ──────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Groups")
    TArray<FCrowd_DinoGroup> RegisteredGroups;

    // ── Max simultaneous active agents (LOD budget) ──────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Performance")
    int32 MaxActiveAgents;

    // ── Tick interval for crowd AI updates (seconds) ─────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Performance")
    float CrowdUpdateInterval;

    // ── Current active agent count (read-only) ───────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Stats",
              meta = (AllowPrivateAccess = "true"))
    int32 ActiveAgentCount;

    // ── Register a new crowd group at runtime ────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterGroup(const FCrowd_DinoGroup& NewGroup);

    // ── Remove a group by ID ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterGroup(const FString& GroupID);

    // ── Get all groups of a given role ───────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_DinoGroup> GetGroupsByRole(ECrowd_DinoRole Role) const;

    // ── Trigger panic response (predator detected) ────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerHerdPanic(const FVector& ThreatLocation, float PanicRadius);

private:
    float TimeSinceLastUpdate;
};
