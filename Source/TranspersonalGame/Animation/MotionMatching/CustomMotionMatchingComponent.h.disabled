#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Engine/DataTable.h"
#include "CustomMotionMatchingComponent.generated.h"

/** Estrutura para definir características de movimento únicas */
USTRUCT(BlueprintType)
struct FMovementCharacteristics
{
    GENERATED_BODY()

    /** Velocidade base de movimento (multiplicador) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseSpeedMultiplier = 1.0f;
    
    /** Tendência para movimentos nervosos (0.0 = calmo, 1.0 = muito nervoso) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float NervousTendency = 0.0f;
    
    /** Preferência por movimentos furtivos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StealthPreference = 0.0f;
    
    /** Confiança em movimentos (afeta postura) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float ConfidenceLevel = 0.5f;
    
    /** Tendência para pausas e hesitações */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float HesitationFrequency = 0.1f;
};

/** Data Table Row para arquétipos de personalidade */
USTRUCT(BlueprintType)
struct FPersonalityArchetypeData : public FTableRowBase
{
    GENERATED_BODY()

    /** Nome do arquétipo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName;
    
    /** Características de movimento para este arquétipo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FMovementCharacteristics MovementTraits;
    
    /** Databases específicas para este arquétipo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    TArray<TSoftObjectPtr<UPoseSearchDatabase>> ArchetypeDatabases;
    
    /** Descrição do comportamento */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FText BehaviorDescription;
};

/**
 * Componente de Motion Matching personalizado
 * Implementa variações únicas de movimento baseadas em personalidade e estado emocional
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCustomMotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCustomMotionMatchingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO DE PERSONALIDADE ===
    
    /** Data Table com arquétipos de personalidade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality System")
    TObjectPtr<UDataTable> PersonalityArchetypesTable;
    
    /** Arquétipo atual do personagem */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality System")
    FName CurrentArchetype = "Cautious";
    
    /** Características de movimento atuais */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Personality System")
    FMovementCharacteristics CurrentMovementTraits;

    // === SISTEMA DE QUERY INTELIGENTE ===
    
    /** Peso para queries de posição (pés, mãos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query Weights", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float PositionQueryWeight = 1.0f;
    
    /** Peso para queries de velocidade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query Weights", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float VelocityQueryWeight = 1.2f;
    
    /** Peso para queries de trajectória */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query Weights", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float TrajectoryQueryWeight = 0.8f;
    
    /** Peso para queries de pose (postura corporal) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query Weights", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float PoseQueryWeight = 1.1f;

    // === DINOSSAURO VARIATIONS ===
    
    /** Variações de tamanho afectam a selecção de animações */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Traits")
    float SizeVariation = 1.0f; // 0.8 = pequeno, 1.2 = grande
    
    /** Idade afecta velocidade e energia */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Traits")
    float AgeMultiplier = 1.0f; // 0.7 = velho, 1.3 = jovem
    
    /** Condição física (ferimentos, fadiga) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Traits")
    float PhysicalCondition = 1.0f; // 0.5 = ferido, 1.0 = saudável
    
    /** Personalidade específica do dinossauro */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Traits")
    FName DinosaurPersonality = "Neutral";

    // === FUNÇÕES PÚBLICAS ===
    
    /** Muda o arquétipo de personalidade */
    UFUNCTION(BlueprintCallable, Category = "Personality System")
    void SetPersonalityArchetype(FName NewArchetype);
    
    /** Obtém as características de movimento atuais */
    UFUNCTION(BlueprintPure, Category = "Personality System")
    FMovementCharacteristics GetCurrentMovementTraits() const { return CurrentMovementTraits; }
    
    /** Calcula peso de query personalizado baseado no contexto */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateContextualQueryWeight(FName QueryType, float BaseWeight) const;
    
    /** Aplica variações procedurais baseadas em traits do dinossauro */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Traits")
    void ApplyDinosaurVariations(int32 Seed, float Size, float Age, float Condition);
    
    /** Verifica se deve aplicar hesitação no movimento */
    UFUNCTION(BlueprintPure, Category = "Movement Behavior")
    bool ShouldApplyHesitation() const;
    
    /** Obtém multiplicador de velocidade baseado no estado atual */
    UFUNCTION(BlueprintPure, Category = "Movement Behavior")
    float GetSpeedMultiplier() const;

private:
    // Estado interno
    float LastHesitationTime = 0.0f;
    float NextHesitationDelay = 0.0f;
    FRandomStream PersonalityRandomStream;
    
    // Funções internas
    void LoadPersonalityData();
    void UpdateMovementTraitsFromArchetype();
    void CalculateNextHesitation();
    float ApplyPersonalityToWeight(float BaseWeight, FName QueryType) const;
};