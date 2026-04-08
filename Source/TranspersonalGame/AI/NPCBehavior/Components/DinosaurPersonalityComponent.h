#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurPersonalityComponent.generated.h"

/**
 * Enumeração para traços de personalidade
 */
UENUM(BlueprintType)
enum class EPersonalityTrait : uint8
{
    // Traços principais (0-100)
    Aggressiveness   UMETA(DisplayName = "Aggressiveness"),
    Curiosity        UMETA(DisplayName = "Curiosity"),
    Sociability      UMETA(DisplayName = "Sociability"),
    Fearfulness      UMETA(DisplayName = "Fearfulness"),
    Territoriality   UMETA(DisplayName = "Territoriality"),
    Intelligence     UMETA(DisplayName = "Intelligence"),
    Playfulness      UMETA(DisplayName = "Playfulness"),
    Stubbornness     UMETA(DisplayName = "Stubbornness"),
    Loyalty          UMETA(DisplayName = "Loyalty"),
    Alertness        UMETA(DisplayName = "Alertness")
};

/**
 * Estrutura para armazenar preferências comportamentais
 */
USTRUCT(BlueprintType)
struct FBehaviorPreferences
{
    GENERATED_BODY()

    // Horários preferidos para diferentes actividades (0.0 = meia-noite, 0.5 = meio-dia)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredFeedingTime = 0.3f; // Manhã cedo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredRestTime = 0.0f; // Meia-noite

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredSocialTime = 0.6f; // Tarde

    // Localizações preferidas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PreferredBiomes; // "Dense Forest", "River Bank", "Open Plains"

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredGroupSize = 3.0f; // Tamanho ideal do grupo

    // Distâncias de conforto
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PersonalSpaceRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ComfortZoneRadius = 1000.0f;

    FBehaviorPreferences()
    {
        PreferredBiomes.Add(TEXT("Mixed Forest"));
    }
};

/**
 * Componente que define a personalidade única de cada dinossauro
 * Influencia todas as decisões comportamentais e reacções
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurPersonalityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurPersonalityComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Traços de personalidade (0-100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality Traits", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    TMap<EPersonalityTrait, float> PersonalityTraits;

    // Preferências comportamentais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Preferences")
    FBehaviorPreferences BehaviorPreferences;

    // Histórico de experiências que moldaram a personalidade
    UPROPERTY(BlueprintReadOnly, Category = "Personality Development")
    TArray<FString> FormativeExperiences;

    // Quirks únicos (comportamentos idiossincráticos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unique Behaviors")
    TArray<FString> PersonalityQuirks;

    // Estado emocional actual
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float CurrentMood = 50.0f; // 0 = muito negativo, 100 = muito positivo

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float StressLevel = 0.0f; // 0 = calmo, 100 = extremamente stressado

    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float EnergyLevel = 100.0f; // 0 = exausto, 100 = cheio de energia

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Personality")
    void GenerateRandomPersonality();

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityTrait(EPersonalityTrait Trait);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void ModifyPersonalityTrait(EPersonalityTrait Trait, float Change);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void AdjustMood(float Change);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void AdjustStress(float Change);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void AdjustEnergy(float Change);

    // Funções de decisão baseadas na personalidade
    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldApproachUnknown(float DistanceToUnknown);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldFleeFromThreat(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldJoinGroup(int32 GroupSize);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    float GetReactionSpeed(); // Baseado em alertness e energia

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    float GetTrustGainRate(); // Baseado em sociability e fearfulness

    // Função para adicionar experiência formativa
    UFUNCTION(BlueprintCallable, Category = "Personality Development")
    void AddFormativeExperience(const FString& Experience);

    // Função para determinar compatibilidade com outro dinossauro
    UFUNCTION(BlueprintCallable, Category = "Social Compatibility")
    float CalculateCompatibility(UDinosaurPersonalityComponent* OtherPersonality);

protected:
    // Função para gerar quirks baseados nos traços de personalidade
    void GeneratePersonalityQuirks();

    // Função para balancear traços de personalidade (evitar extremos irrealistas)
    void BalancePersonalityTraits();

    // Timer para decay natural de emoções
    FTimerHandle EmotionalDecayTimer;

    UFUNCTION()
    void ProcessEmotionalDecay();

private:
    // Função para gerar traço individual
    float GenerateTraitValue(EPersonalityTrait Trait);

    // Função para determinar se dois traços são incompatíveis
    bool AreTraitsIncompatible(EPersonalityTrait TraitA, EPersonalityTrait TraitB);
};