#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearch.h"
#include "Animation/MotionMatchingAnimInstance.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

/**
 * Sistema central de animação para o jogo Transpersonal
 * Gerencia Motion Matching, IK adaptativo e variações procedurais
 * Baseado nos princípios de Richard Williams e na técnica do RDR2
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === MOTION MATCHING CORE ===
    
    /** Schema principal para Motion Matching do protagonista */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* ProtagonistMotionSchema;
    
    /** Database de animações do protagonista (cauteloso, científico) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* ProtagonistAnimDatabase;
    
    /** Schema para dinossauros herbívoros pequenos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* SmallHerbivoreSchema;
    
    /** Schema para dinossauros carnívoros */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* CarnivoreSchema;
    
    // === IK SYSTEM ===
    
    /** IK Rig para adaptação de terreno (pés) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    class UIKRigDefinition* TerrainAdaptationRig;
    
    /** IK Rig para interações com objectos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    class UIKRigDefinition* InteractionRig;
    
    // === VARIATION SYSTEM ===
    
    /** Multiplicador de variação para dinossauros únicos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float DinosaurVariationMultiplier = 1.0f;
    
    /** Seed para variações procedurais */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    int32 VariationSeed = 12345;

public:
    // === ANIMATION FUNCTIONS ===
    
    /** Inicializa Motion Matching para um personagem específico */
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    bool InitializeMotionMatching(AActor* Character, ECharacterType CharacterType);
    
    /** Aplica variações únicas a um dinossauro */
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void ApplyDinosaurVariations(AActor* Dinosaur, FString DinosaurID);
    
    /** Actualiza IK para adaptação ao terreno */
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateTerrainAdaptation(AActor* Character, float DeltaTime);
    
    /** Calcula blend weight baseado no estado emocional */
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    float CalculateEmotionalBlendWeight(float FearLevel, float CuriosityLevel);

protected:
    // === INTERNAL FUNCTIONS ===
    
    /** Configura trajectory sampling para Motion Matching */
    void SetupTrajectorySystem();
    
    /** Inicializa pose search databases */
    void InitializePoseSearchDatabases();
    
    /** Aplica modificadores de personalidade às animações */
    void ApplyPersonalityModifiers(AActor* Character, FString PersonalityProfile);

private:
    // === RUNTIME DATA ===
    
    /** Cache de Motion Matching instances activas */
    UPROPERTY()
    TMap<AActor*, class UMotionMatchingAnimInstance*> ActiveMotionMatchingInstances;
    
    /** Cache de IK Rig instances */
    UPROPERTY()
    TMap<AActor*, class UIKRigProcessor*> ActiveIKProcessors;
    
    /** Dados de variação por dinossauro */
    UPROPERTY()
    TMap<FString, FDinosaurAnimationVariation> DinosaurVariations;
};

/** Tipos de personagem suportados */
UENUM(BlueprintType)
enum class ECharacterType : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonista Paleontologista"),
    SmallHerbivore  UMETA(DisplayName = "Herbívoro Pequeno"),
    LargeHerbivore  UMETA(DisplayName = "Herbívoro Grande"),
    SmallCarnivore  UMETA(DisplayName = "Carnívoro Pequeno"),
    LargeCarnivore  UMETA(DisplayName = "Carnívoro Grande"),
    FlyingCreature  UMETA(DisplayName = "Criatura Voadora")
};

/** Estrutura de variação para dinossauros únicos */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurAnimationVariation
{
    GENERATED_BODY()

    /** Modificador de velocidade de movimento */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeedModifier = 1.0f;
    
    /** Modificador de postura (mais erecto/curvado) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PostureModifier = 0.0f;
    
    /** Modificador de agressividade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionModifier = 0.0f;
    
    /** Modificador de nervosismo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NervousnessModifier = 0.0f;
    
    /** Padrão de respiração único */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreathingPattern = 1.0f;
    
    /** Frequência de movimentos de cabeça */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeadMovementFrequency = 1.0f;
};