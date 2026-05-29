#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TribalDefenseSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_DefenseFormation : uint8
{
    Circle      UMETA(DisplayName = "Circle Formation"),
    Line        UMETA(DisplayName = "Line Formation"),
    Wedge       UMETA(DisplayName = "Wedge Formation"),
    Scatter     UMETA(DisplayName = "Scatter Formation"),
    Retreat     UMETA(DisplayName = "Retreat Formation")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DefensePosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    bool bIsOccupied;

    FCombat_DefensePosition()
    {
        Position = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Priority = 1;
        bIsOccupied = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    ECombat_DefenseFormation PreferredFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    float ResponseTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    int32 MinDefenders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    bool bRequiresEvacuation;

    FCombat_ThreatResponse()
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        PreferredFormation = ECombat_DefenseFormation::Circle;
        ResponseTime = 5.0f;
        MinDefenders = 2;
        bRequiresEvacuation = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TribalDefenseSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TribalDefenseSystem();

protected:
    virtual void BeginPlay() override;

    // Defense Formation Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Formation")
    ECombat_DefenseFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Formation")
    TArray<FCombat_DefensePosition> DefensePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Formation")
    int32 MaxDefenders;

    // Threat Assessment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    TArray<FCombat_ThreatResponse> ThreatResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatUpdateInterval;

    // Communication System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    bool bUseVisualSignals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    bool bUseAudioSignals;

    // Tribal Members
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Members")
    TArray<AActor*> TribalMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Members")
    TArray<AActor*> ActiveDefenders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Members")
    AActor* DefenseLeader;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Defense Formation Functions
    UFUNCTION(BlueprintCallable, Category = "Defense Formation")
    void SetDefenseFormation(ECombat_DefenseFormation NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Defense Formation")
    void GenerateDefensePositions();

    UFUNCTION(BlueprintCallable, Category = "Defense Formation")
    FVector GetOptimalDefensePosition(AActor* Defender);

    UFUNCTION(BlueprintCallable, Category = "Defense Formation")
    bool AssignDefensePosition(AActor* Defender, const FVector& Position);

    // Threat Management Functions
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void AssessThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void RespondToThreat(ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    TArray<AActor*> DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    ECombat_ThreatLevel CalculateThreatLevel(const TArray<AActor*>& Threats);

    // Communication Functions
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastAlert(ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendFormationCommand(ECombat_DefenseFormation Formation);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void AlertNearbyTribes(const FVector& ThreatLocation);

    // Tribal Management Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Management")
    void RegisterTribalMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Tribal Management")
    void UnregisterTribalMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Tribal Management")
    void AssignDefenseRoles();

    UFUNCTION(BlueprintCallable, Category = "Tribal Management")
    bool CanDefendAgainstThreat(ECombat_ThreatLevel ThreatLevel);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsPositionSafe(const FVector& Position);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetSafeRetreatPosition();

private:
    // Internal state management
    float LastThreatUpdate;
    bool bInCombat;
    FVector LastKnownThreatLocation;
    
    // Helper functions
    void UpdateDefensePositions();
    void CheckFormationIntegrity();
    void HandleEmergencyEvacuation();
    ECombat_DefenseFormation SelectOptimalFormation(const TArray<AActor*>& Threats);
};