#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "ArchetypalSystem.generated.h"

UENUM(BlueprintType)
enum class EArchetypalRole : uint8
{
    Hero            UMETA(DisplayName = "The Hero"),
    Sage            UMETA(DisplayName = "The Sage"),
    Innocent        UMETA(DisplayName = "The Innocent"),
    Explorer        UMETA(DisplayName = "The Explorer"),
    Rebel           UMETA(DisplayName = "The Rebel"),
    Magician        UMETA(DisplayName = "The Magician"),
    Everyman        UMETA(DisplayName = "The Everyman"),
    Lover           UMETA(DisplayName = "The Lover"),
    Jester          UMETA(DisplayName = "The Jester"),
    Caregiver       UMETA(DisplayName = "The Caregiver"),
    Ruler           UMETA(DisplayName = "The Ruler"),
    Creator         UMETA(DisplayName = "The Creator"),
    Shadow          UMETA(DisplayName = "The Shadow"),
    Anima           UMETA(DisplayName = "The Anima"),
    Animus          UMETA(DisplayName = "The Animus"),
    Self            UMETA(DisplayName = "The Self")
};

UENUM(BlueprintType)
enum class EArchetypalEnergy : uint8
{
    Masculine       UMETA(DisplayName = "Masculine Energy"),
    Feminine        UMETA(DisplayName = "Feminine Energy"),
    Neutral         UMETA(DisplayName = "Neutral Energy"),
    Transcendent    UMETA(DisplayName = "Transcendent Energy"),
    Shadow          UMETA(DisplayName = "Shadow Energy"),
    Light           UMETA(DisplayName = "Light Energy")
};

USTRUCT(BlueprintType)
struct FArchetypalProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    EArchetypalRole PrimaryArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    EArchetypalRole SecondaryArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    EArchetypalEnergy DominantEnergy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float Integration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float ShadowWork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    TMap<EArchetypalRole, float> ArchetypalResonance;

    FArchetypalProfile()
    {
        PrimaryArchetype = EArchetypalRole::Hero;
        SecondaryArchetype = EArchetypalRole::Innocent;
        DominantEnergy = EArchetypalEnergy::Neutral;
        Integration = 50.0f;
        ShadowWork = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FArchetypalInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EArchetypalRole Archetype1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EArchetypalRole Archetype2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float Resonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float Tension;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float TransformationPotential;

    FArchetypalInteraction()
    {
        Archetype1 = EArchetypalRole::Hero;
        Archetype2 = EArchetypalRole::Shadow;
        Resonance = 0.0f;
        Tension = 0.0f;
        TransformationPotential = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchetypalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchetypalComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FArchetypalProfile Profile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float ArchetypalPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    bool bShadowIntegrated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float IndividuationProgress;

    UFUNCTION(BlueprintCallable, Category = "Archetype")
    void ActivateArchetype(EArchetypalRole Archetype);

    UFUNCTION(BlueprintCallable, Category = "Archetype")
    void BeginShadowWork();

    UFUNCTION(BlueprintCallable, Category = "Archetype")
    void IntegrateArchetype(EArchetypalRole Archetype);

    UFUNCTION(BlueprintCallable, Category = "Archetype")
    FArchetypalInteraction CalculateInteraction(UArchetypalComponent* OtherComponent);

    UFUNCTION(BlueprintCallable, Category = "Archetype")
    void ProcessIndividuation(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Archetype")
    float GetArchetypalResonance(EArchetypalRole Archetype) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnArchetypalActivation(EArchetypalRole Archetype);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnShadowEncounter();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnIndividuationMilestone(float Progress);

private:
    UConsciousnessComponent* ConsciousnessComp;
    float ActivationTimer;
    TArray<EArchetypalRole> ActiveArchetypes;

    void UpdateArchetypalEnergies(float DeltaTime);
    void ProcessShadowIntegration(float DeltaTime);
    void CalculateArchetypalResonances();
    float GetArchetypalCompatibility(EArchetypalRole Arch1, EArchetypalRole Arch2) const;
};

UCLASS()
class TRANSPERSONALGAME_API AArchetypalGuide : public ACharacter
{
    GENERATED_BODY()

public:
    AArchetypalGuide();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guide")
    EArchetypalRole GuideArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guide")
    float WisdomLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guide")
    float GuidanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> WisdomQuotes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ChallengeQuestions;

    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void OfferGuidance(AActor* Seeker);

    UFUNCTION(BlueprintCallable, Category = "Guidance")
    void PresentChallenge(AActor* Seeker, EArchetypalRole ChallengeType);

    UFUNCTION(BlueprintCallable, Category = "Guidance")
    FString GetWisdomForArchetype(EArchetypalRole Archetype);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnSeekerApproach(AActor* Seeker);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnGuidanceComplete(AActor* Seeker, bool bSuccessful);

private:
    TArray<AActor*> CurrentSeekers;
    float GuidanceTimer;

    void UpdateGuidanceField(float DeltaTime);
    bool IsArchetypeCompatible(EArchetypalRole SeekerArchetype) const;
};

UCLASS()
class TRANSPERSONALGAME_API UArchetypalManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Management")
    TMap<EArchetypalRole, float> GlobalArchetypalField;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Management")
    float CollectiveIndividuation;

    UFUNCTION(BlueprintCallable, Category = "Management")
    void UpdateGlobalField();

    UFUNCTION(BlueprintCallable, Category = "Management")
    void RegisterArchetypalComponent(UArchetypalComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Management")
    void UnregisterArchetypalComponent(UArchetypalComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Management")
    TArray<UArchetypalComponent*> FindCompatibleArchetypes(EArchetypalRole Archetype, float MinResonance = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Management")
    void TriggerCollectiveEvent(EArchetypalRole Archetype, float Intensity);

    UFUNCTION(BlueprintPure, Category = "Management")
    float GetArchetypalFieldStrength(EArchetypalRole Archetype) const;

private:
    TArray<UArchetypalComponent*> RegisteredComponents;
    float FieldUpdateTimer;

    void CalculateCollectiveIndividuation();
    void ProcessArchetypalInteractions();
};