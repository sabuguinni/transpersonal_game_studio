// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
// FIXME: Missing header - #include "NPCBehaviorCore.h"
#include "SocialInteractionSystem.generated.h"

UENUM(BlueprintType)
enum class ESocialInteractionType : uint8
{
    Greeting        UMETA(DisplayName = "Greeting"),
    Grooming        UMETA(DisplayName = "Grooming"),
    PlayFight       UMETA(DisplayName = "Play Fight"),
    Mating          UMETA(DisplayName = "Mating"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Feeding         UMETA(DisplayName = "Feeding Together"),
    Warning         UMETA(DisplayName = "Warning"),
    Submission      UMETA(DisplayName = "Submission"),
    Dominance       UMETA(DisplayName = "Dominance Display"),
    Comfort         UMETA(DisplayName = "Comfort")
};

UENUM(BlueprintType)
enum class ESocialHierarchy : uint8
{
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Omega           UMETA(DisplayName = "Omega"),
    Juvenile        UMETA(DisplayName = "Juvenile"),
    Elder           UMETA(DisplayName = "Elder"),
    Outcast         UMETA(DisplayName = "Outcast")
};

USTRUCT(BlueprintType)
struct FSocialInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* InitiatorActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESocialInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector InteractionLocation;

    FSocialInteraction()
    {
        InitiatorActor = nullptr;
        TargetActor = nullptr;
        InteractionType = ESocialInteractionType::Greeting;
        Duration = 5.0f;
        IntensityLevel = 0.5f;
        StartTime = 0.0f;
        bIsActive = false;
        InteractionLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FSocialGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* AlphaLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector GroupCenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupCohesion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GroupName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNomadic;

    FSocialGroup()
    {
        AlphaLeader = nullptr;
        GroupCenterLocation = FVector::ZeroVector;
        GroupCohesion = 0.5f;
        GroupRadius = 500.0f;
        GroupName = TEXT("Unnamed Group");
        bIsNomadic = false;
    }
};

USTRUCT(BlueprintType)
struct FSocialProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESocialHierarchy HierarchyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Dominance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Empathy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Loyalty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<ESocialInteractionType, float> InteractionPreferences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> PreferredCompanions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> Rivals;

    FSocialProfile()
    {
        HierarchyLevel = ESocialHierarchy::Beta;
        Dominance = 0.5f;
        Sociability = 0.5f;
        Aggression = 0.3f;
        Empathy = 0.4f;
        Loyalty = 0.6f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USocialInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    USocialInteractionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Social Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    FSocialProfile SocialProfile;

// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    FSocialGroup* CurrentGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    float SocialEnergy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Profile")
    float SocialStress;

    // Interaction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float MaxInteractionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float SocialScanRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float InteractionCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    int32 MaxSimultaneousInteractions;

    // Current Interactions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    TArray<FSocialInteraction> ActiveInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    TArray<AActor*> NearbyNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float LastInteractionTime;

    // Group Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Management")
    bool bCanFormGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Management")
    bool bCanLeadGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Management")
    float GroupFormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Management")
    int32 PreferredGroupSize;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    TArray<FString> VocalizationSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    bool bCanUseBodyLanguage;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    bool InitiateInteraction(AActor* TargetActor, ESocialInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void EndInteraction(const FSocialInteraction& Interaction);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    bool CanInteractWith(AActor* TargetActor, ESocialInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void UpdateSocialRelationships();

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    ESocialInteractionType DetermineInteractionType(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void RespondToInteraction(const FSocialInteraction& Interaction, bool bAccept);

// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Group Management")
    void JoinGroup(FSocialGroup* Group);

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void LeaveGroup();

// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Group Management")
    FSocialGroup* CreateNewGroup();

    UFUNCTION(BlueprintCallable, Category = "Group Management")
    void UpdateGroupDynamics();

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void Vocalize(const FString& SoundName, float Range = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void PerformBodyLanguage(ESocialInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social Profile")
    void UpdateHierarchyLevel();

    UFUNCTION(BlueprintCallable, Category = "Social Profile")
    float CalculateCompatibility(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Social Profile")
    void ModifySocialTrait(const FString& TraitName, float Change);

private:
    void ScanForNearbyNPCs();
    void ProcessActiveInteractions(float DeltaTime);
    void EvaluateInteractionOpportunities();
    void UpdateSocialEnergy(float DeltaTime);
    void HandleGroupBehavior();
    
    float CalculateInteractionSuccess(AActor* TargetActor, ESocialInteractionType InteractionType);
    bool IsInInteractionCooldown();
    void ApplyInteractionEffects(const FSocialInteraction& Interaction);
    
    // Internal state
    float LastSocialScan;
    float LastGroupUpdate;
    TArray<AActor*> RecentInteractionPartners;
};