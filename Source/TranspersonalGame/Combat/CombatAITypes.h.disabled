#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "CombatAITypes.generated.h"

/**
 * Combat AI threat assessment levels for tactical decision making
 */
UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

/**
 * Combat AI tactical states for coordinated behavior
 */
UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol      UMETA(DisplayName = "Patrolling"),
    Scout       UMETA(DisplayName = "Scouting"),
    Stalk       UMETA(DisplayName = "Stalking"),
    Coordinate  UMETA(DisplayName = "Coordinating"),
    Engage      UMETA(DisplayName = "Engaging"),
    Flank       UMETA(DisplayName = "Flanking"),
    Retreat     UMETA(DisplayName = "Retreating"),
    Regroup     UMETA(DisplayName = "Regrouping"),
    Defend      UMETA(DisplayName = "Defending"),
    Ambush      UMETA(DisplayName = "Ambushing")
};

/**
 * Combat AI formation types for pack coordination
 */
UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None        UMETA(DisplayName = "No Formation"),
    Line        UMETA(DisplayName = "Line Formation"),
    Circle      UMETA(DisplayName = "Circle Formation"),
    Wedge       UMETA(DisplayName = "Wedge Formation"),
    Pincer      UMETA(DisplayName = "Pincer Formation"),
    Ambush      UMETA(DisplayName = "Ambush Formation"),
    Scatter     UMETA(DisplayName = "Scatter Formation")
};

/**
 * Combat AI attack patterns for varied engagement
 */
UENUM(BlueprintType)
enum class ECombat_AttackPattern : uint8
{
    Direct      UMETA(DisplayName = "Direct Attack"),
    Flanking    UMETA(DisplayName = "Flanking Attack"),
    Hit_Run     UMETA(DisplayName = "Hit and Run"),
    Coordinated UMETA(DisplayName = "Coordinated Strike"),
    Ambush      UMETA(DisplayName = "Ambush Attack"),
    Feint       UMETA(DisplayName = "Feint Attack"),
    Overwhelm   UMETA(DisplayName = "Overwhelm Attack")
};

/**
 * Combat AI pack roles for specialized behavior
 */
UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Leader      UMETA(DisplayName = "Pack Leader"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Scout       UMETA(DisplayName = "Scout"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Guardian    UMETA(DisplayName = "Guardian"),
    Berserker   UMETA(DisplayName = "Berserker"),
    Support     UMETA(DisplayName = "Support")
};

/**
 * Combat target information for AI decision making
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TargetInfo
{
    GENERATED_BODY()

    /** Target actor reference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    TWeakObjectPtr<AActor> Target;

    /** Threat level assessment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    ECombat_ThreatLevel ThreatLevel;

    /** Distance to target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    float Distance;

    /** Last known position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    FVector LastKnownPosition;

    /** Time since last seen */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    float TimeSinceLastSeen;

    /** Target health percentage (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    float HealthPercentage;

    /** Is target currently visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    bool bIsVisible;

    /** Constructor */
    FCombat_TargetInfo()
    {
        Target = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastKnownPosition = FVector::ZeroVector;
        TimeSinceLastSeen = 0.0f;
        HealthPercentage = 1.0f;
        bIsVisible = false;
    }
};

/**
 * Combat AI tactical parameters for decision making
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalParams
{
    GENERATED_BODY()

    /** Aggression level (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel;

    /** Coordination skill (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CoordinationSkill;

    /** Preferred engagement distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    float PreferredEngagementDistance;

    /** Retreat health threshold (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RetreatHealthThreshold;

    /** Pack loyalty factor (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackLoyalty;

    /** Territorial behavior strength (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerritorialBehavior;

    /** Constructor */
    FCombat_TacticalParams()
    {
        AggressionLevel = 0.5f;
        CoordinationSkill = 0.5f;
        PreferredEngagementDistance = 500.0f;
        RetreatHealthThreshold = 0.2f;
        PackLoyalty = 0.7f;
        TerritorialBehavior = 0.6f;
    }
};

/**
 * Combat AI pack coordination data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackCoordination
{
    GENERATED_BODY()

    /** Pack leader reference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    TWeakObjectPtr<APawn> PackLeader;

    /** Pack members */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    TArray<TWeakObjectPtr<APawn>> PackMembers;

    /** Current formation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    ECombat_Formation CurrentFormation;

    /** Formation center point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    FVector FormationCenter;

    /** Pack morale (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackMorale;

    /** Coordination effectiveness (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CoordinationEffectiveness;

    /** Constructor */
    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        PackMembers.Empty();
        CurrentFormation = ECombat_Formation::None;
        FormationCenter = FVector::ZeroVector;
        PackMorale = 1.0f;
        CoordinationEffectiveness = 0.5f;
    }
};

/**
 * Combat AI attack data for coordinated strikes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AttackData
{
    GENERATED_BODY()

    /** Attack pattern */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Attack")
    ECombat_AttackPattern AttackPattern;

    /** Attack target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Attack")
    TWeakObjectPtr<AActor> AttackTarget;

    /** Attack position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Attack")
    FVector AttackPosition;

    /** Attack timing (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Attack")
    float AttackTiming;

    /** Expected damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Attack")
    float ExpectedDamage;

    /** Success probability (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Attack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SuccessProbability;

    /** Constructor */
    FCombat_AttackData()
    {
        AttackPattern = ECombat_AttackPattern::Direct;
        AttackTarget = nullptr;
        AttackPosition = FVector::ZeroVector;
        AttackTiming = 0.0f;
        ExpectedDamage = 0.0f;
        SuccessProbability = 0.5f;
    }
};