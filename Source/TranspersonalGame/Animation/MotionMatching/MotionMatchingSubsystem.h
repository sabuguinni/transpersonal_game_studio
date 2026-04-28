#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
// DISABLED: #include "Animation/PoseSearch/PoseSearchDatabase.h"
// DISABLED: #include "Animation/PoseSearch/PoseSearchSchema.h"
#include "Animation/AnimInstance.h"
#include "MotionMatchingSubsystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MotionMatchingContext : uint8
{
    Locomotion          UMETA(DisplayName = "Locomotion"),
    Stealth            UMETA(DisplayName = "Stealth"),
    Panic              UMETA(DisplayName = "Panic"),
    Exhausted          UMETA(DisplayName = "Exhausted"),
    Injured            UMETA(DisplayName = "Injured"),
    Gathering          UMETA(DisplayName = "Gathering"),
    Climbing           UMETA(DisplayName = "Climbing"),
    Swimming           UMETA(DisplayName = "Swimming"),
    Interaction        UMETA(DisplayName = "Interaction")
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatchingQuery
{
    GENERATED_BODY()

    // Trajectory data
    UPROPERTY(BlueprintReadWrite, Category = "Trajectory")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Trajectory")
    FVector DesiredVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Trajectory")
    FVector FuturePosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Trajectory")
    float TurnRate = 0.0f;

    // Emotional modifiers
    UPROPERTY(BlueprintReadWrite, Category = "Emotion")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Emotion")
    float ExhaustionLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Emotion")
    float InjuryLevel = 0.0f;

    // Environmental context
    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    float TerrainSlope = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    bool bIsInDanger = false;

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    bool bIsHiding = false;

    // Activity context
    UPROPERTY(BlueprintReadWrite, Category = "Activity")
    EAnim_MotionMatchingContext Context = EAnim_MotionMatchingContext::Locomotion;
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatchingDatabaseSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> StealthDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> PanicDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> ExhaustedDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> InjuredDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> GatheringDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> ClimbingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> SwimmingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TObjectPtr<UPoseSearchDatabase> InteractionDatabase;
};

/**
 * Subsistema central para Motion Matching no jogo
 * Gere a seleção inteligente de bases de dados de animação baseada no contexto emocional e ambiental
 * Cada personagem tem uma linguagem corporal única que reflete a sua personalidade e estado
 */
UCLASS()
class TRANSPERSONALGAME_API UMotionMatchingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Seleciona a base de dados de animação mais apropriada baseada no query
     * @param Query - Dados do contexto actual do personagem
     * @param CharacterType - Tipo de personagem (Player, NPC, Dinosaur)
     * @return Base de dados de pose search mais apropriada
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* SelectOptimalDatabase(const FAnim_MotionMatchingQuery& Query, const FString& CharacterType = "Player");

    /**
     * Calcula o blend time ideal entre animações baseado no contexto emocional
     * @param FromContext - Contexto anterior
     * @param ToContext - Contexto novo
     * @param EmotionalIntensity - Intensidade da mudança emocional (0.0-1.0)
     * @return Tempo de blend em segundos
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateOptimalBlendTime(EAnim_MotionMatchingContext FromContext, EAnim_MotionMatchingContext ToContext, float EmotionalIntensity = 0.5f);

    /**
     * Regista uma nova base de dados para um tipo de personagem
     * @param CharacterType - Tipo de personagem
     * @param DatabaseSet - Conjunto de bases de dados para diferentes contextos
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void RegisterCharacterDatabases(const FString& CharacterType, const FAnim_MotionMatchingDatabaseSet& DatabaseSet);

    /**
     * Aplica modificadores emocionais ao query de motion matching
     * @param BaseQuery - Query base
     * @param FearLevel - Nível de medo (0.0-1.0)
     * @param ExhaustionLevel - Nível de exaustão (0.0-1.0)
     * @param InjuryLevel - Nível de ferimentos (0.0-1.0)
     * @return Query modificado com factores emocionais
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionMatchingQuery ApplyEmotionalModifiers(const FAnim_MotionMatchingQuery& BaseQuery, float FearLevel, float ExhaustionLevel, float InjuryLevel);

protected:
    // Mapeamento de tipos de personagem para as suas bases de dados
    UPROPERTY()
    TMap<FString, FAnim_MotionMatchingDatabaseSet> CharacterDatabases;

    // Configurações de blend time por contexto
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EAnim_MotionMatchingContext, float> ContextBlendTimes;

    // Pesos para diferentes factores emocionais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float FearInfluenceWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ExhaustionInfluenceWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float InjuryInfluenceWeight = 0.9f;

private:
    void InitializeDefaultBlendTimes();
    EAnim_MotionMatchingContext DetermineOptimalContext(const FAnim_MotionMatchingQuery& Query);
    float CalculateContextPriority(EAnim_MotionMatchingContext Context, const FAnim_MotionMatchingQuery& Query);
};