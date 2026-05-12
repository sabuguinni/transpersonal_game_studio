#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Combat_SurvivalAIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Extreme     UMETA(DisplayName = "Extreme Threat"),
    Apex        UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class ECombat_EnvironmentType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyTerrain    UMETA(DisplayName = "Rocky Terrain"),
    WaterEdge       UMETA(DisplayName = "Water Edge"),
    CaveSystem      UMETA(DisplayName = "Cave System"),
    SwampLand       UMETA(DisplayName = "Swamp Land")
};

UENUM(BlueprintType)
enum class ECombat_AIBehaviorState : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Injured         UMETA(DisplayName = "Injured")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EnvironmentalFactor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    ECombat_EnvironmentType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float VisibilityModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbushBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 EscapeRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MovementSpeedModifier;

    FCombat_EnvironmentalFactor()
    {
        EnvironmentType = ECombat_EnvironmentType::OpenPlains;
        VisibilityModifier = 1.0f;
        AmbushBonus = 1.0f;
        EscapeRoutes = 4;
        MovementSpeedModifier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    bool bCallForBackup;

    FCombat_ThreatResponse()
    {
        ThreatLevel = ECombat_ThreatLevel::Low;
        DetectionRange = 800.0f;
        AggressionLevel = 0.5f;
        FleeThreshold = 0.6f;
        bCallForBackup = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_SurvivalBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    FString BehaviorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    ECombat_AIBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    bool bRequiresPackCoordination;

    FCombat_SurvivalBehavior()
    {
        BehaviorName = TEXT("Default Behavior");
        TriggerCondition = TEXT("None");
        BehaviorState = ECombat_AIBehaviorState::Passive;
        Duration = 30.0f;
        Intensity = 1.0f;
        bRequiresPackCoordination = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_SurvivalAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_SurvivalAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core survival AI management
    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void InitializeSurvivalAI();

    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void UpdateThreatAssessment(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    ECombat_ThreatLevel CalculateThreatLevel(AActor* PlayerActor, AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void TriggerSurvivalBehavior(const FString& BehaviorName, AActor* TargetActor);

    // Environmental awareness
    UFUNCTION(BlueprintCallable, Category = "Environment")
    ECombat_EnvironmentType DetectEnvironmentType(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FCombat_EnvironmentalFactor GetEnvironmentalFactors(ECombat_EnvironmentType EnvironmentType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float CalculateAmbushProbability(FVector Location, ECombat_EnvironmentType Environment);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackHunt(TArray<AActor*> PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void AssignPackRoles(TArray<AActor*> PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void ExecutePackTactic(const FString& TacticName, TArray<AActor*> PackMembers);

    // Threat communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastThreatAlert(FVector Location, ECombat_ThreatLevel ThreatLevel, float Range);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendPackCall(AActor* CallerActor, FVector Location, float Range);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ProcessDistressCall(AActor* DistressedActor);

    // Survival metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void RecordCombatEvent(const FString& EventType, AActor* Participant1, AActor* Participant2);

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateSurvivalMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float GetPlayerSurvivalTime() const { return PlayerSurvivalTime; }

protected:
    // Environmental factors database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TMap<ECombat_EnvironmentType, FCombat_EnvironmentalFactor> EnvironmentalFactors;

    // Threat response configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Response")
    TMap<ECombat_ThreatLevel, FCombat_ThreatResponse> ThreatResponses;

    // Survival behaviors database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behaviors")
    TArray<FCombat_SurvivalBehavior> SurvivalBehaviors;

    // Active AI actors
    UPROPERTY(BlueprintReadOnly, Category = "AI Management")
    TArray<AActor*> ActiveDinosaurAI;

    UPROPERTY(BlueprintReadOnly, Category = "AI Management")
    TArray<AActor*> ActivePackGroups;

    // Survival metrics
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PlayerSurvivalTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CombatEncounters;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 SuccessfulEscapes;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 PlayerDeaths;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ThreatAssessmentInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxPackCoordinationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAdvancedAI;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePackHunting;

private:
    float LastThreatAssessmentTime;
    bool bSurvivalAIInitialized;

    void InitializeEnvironmentalFactors();
    void InitializeThreatResponses();
    void InitializeSurvivalBehaviors();
    void FindActiveAIActors();
    void ProcessEnvironmentalAwareness();
    void UpdatePackCoordination();
    void HandleThreatCommunication();
};