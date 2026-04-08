#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimationTypes.generated.h"

/**
 * Enum para estados emocionais que afetam a animação
 * Baseado no conceito de "medo constante" do jogo
 */
UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Cautious    UMETA(DisplayName = "Cautious"), 
    Nervous     UMETA(DisplayName = "Nervous"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Exhausted   UMETA(DisplayName = "Exhausted")
};

/**
 * Enum para tipos de terreno que afetam IK de pés
 */
UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    Flat        UMETA(DisplayName = "Flat"),
    Rocky       UMETA(DisplayName = "Rocky"),
    Muddy       UMETA(DisplayName = "Muddy"),
    Steep       UMETA(DisplayName = "Steep"),
    Unstable    UMETA(DisplayName = "Unstable")
};

/**
 * Struct para configuração de Motion Matching por arquétipo
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMotionMatchingConfig : public FTableRowBase
{
    GENERATED_BODY()

    // Nome do arquétipo (Paleontologist, Herbivore, Predator, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ArchetypeName;

    // Database de animações para Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* PoseDatabase;

    // Peso da velocidade na query
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VelocityWeight = 0.7f;

    // Peso da direção na query
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirectionWeight = 0.8f;

    // Peso do estado emocional
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalWeight = 0.5f;

    // Tempo de blend entre poses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float BlendTime = 0.3f;

    FMotionMatchingConfig()
    {
        ArchetypeName = TEXT("Default");
        PoseDatabase = nullptr;
        VelocityWeight = 0.7f;
        DirectionWeight = 0.8f;
        EmotionalWeight = 0.5f;
        BlendTime = 0.3f;
    }
};

/**
 * Struct para configuração de IK de pés
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFootIKConfig
{
    GENERATED_BODY()

    // Distância máxima de trace para o chão
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TraceDistance = 50.0f;

    // Offset vertical do pé
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FootOffset = 5.0f;

    // Velocidade de interpolação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InterpSpeed = 15.0f;

    // Nome do bone do pé
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FootBoneName;

    // Nome do bone do joelho
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName KneeBoneName;

    FFootIKConfig()
    {
        TraceDistance = 50.0f;
        FootOffset = 5.0f;
        InterpSpeed = 15.0f;
        FootBoneName = NAME_None;
        KneeBoneName = NAME_None;
    }
};

/**
 * Struct para dados de animação contextual
 * Cada personagem tem uma "linguagem corporal" única
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterAnimationProfile
{
    GENERATED_BODY()

    // Modificador de postura (0 = curvado, 1 = ereto)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PostureModifier = 0.5f;

    // Modificador de velocidade de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float MovementSpeedModifier = 1.0f;

    // Modificador de nervosismo (afeta tremor e hesitação)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NervousnessModifier = 0.3f;

    // Modificador de fadiga
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FatigueModifier = 0.0f;

    FCharacterAnimationProfile()
    {
        PostureModifier = 0.5f;
        MovementSpeedModifier = 1.0f;
        NervousnessModifier = 0.3f;
        FatigueModifier = 0.0f;
    }
};