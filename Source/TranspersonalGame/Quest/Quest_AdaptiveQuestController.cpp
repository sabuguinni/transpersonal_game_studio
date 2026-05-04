#include "Quest_AdaptiveQuestController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

AQuest_AdaptiveQuestController::AQuest_AdaptiveQuestController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxActiveQuests = 3;
    QuestGenerationInterval = 120.0f; // 2 minutos
    DifficultyAdaptationRate = 0.1f;
    
    // Inicializar dados do jogador
    PlayerSkills = FQuest_PlayerSkillData();
}

void AQuest_AdaptiveQuestController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeQuestTemplates();
    
    // Configurar timer para geração automática de quests
    GetWorld()->GetTimerManager().SetTimer(
        QuestGenerationTimer,
        this,
        &AQuest_AdaptiveQuestController::GenerateAdaptiveQuest,
        QuestGenerationInterval,
        true
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Adaptive Controller iniciado - Geração automática a cada %.1f segundos"), QuestGenerationInterval);
}

void AQuest_AdaptiveQuestController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Verificar timeout das quests activas
    for (int32 i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        if (ActiveQuests[i].bIsActive)
        {
            ActiveQuests[i].TimeLimit -= DeltaTime;
            if (ActiveQuests[i].TimeLimit <= 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest '%s' expirou por timeout"), *ActiveQuests[i].QuestName);
                CompleteQuest(ActiveQuests[i].QuestName, false);
            }
        }
    }
}

void AQuest_AdaptiveQuestController::GenerateAdaptiveQuest()
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Log, TEXT("Máximo de quests activas atingido (%d)"), MaxActiveQuests);
        return;
    }
    
    // Escolher categoria baseada nas skills do jogador
    TArray<EQuest_QuestCategory> Categories = {
        EQuest_QuestCategory::Survival,
        EQuest_QuestCategory::Hunting,
        EQuest_QuestCategory::Exploration,
        EQuest_QuestCategory::Crafting
    };
    
    EQuest_QuestCategory SelectedCategory = Categories[FMath::RandRange(0, Categories.Num() - 1)];
    EQuest_QuestDifficulty OptimalDifficulty = CalculateOptimalDifficulty(SelectedCategory);
    
    FQuest_AdaptiveQuestData NewQuest;
    
    switch (SelectedCategory)
    {
        case EQuest_QuestCategory::Survival:
            NewQuest = CreateSurvivalQuest(OptimalDifficulty);
            break;
        case EQuest_QuestCategory::Hunting:
            NewQuest = CreateHuntingQuest(OptimalDifficulty);
            break;
        case EQuest_QuestCategory::Exploration:
            NewQuest = CreateExplorationQuest(OptimalDifficulty);
            break;
        case EQuest_QuestCategory::Crafting:
            NewQuest = CreateCraftingQuest(OptimalDifficulty);
            break;
        default:
            NewQuest = CreateSurvivalQuest(OptimalDifficulty);
            break;
    }
    
    if (StartQuest(NewQuest))
    {
        UE_LOG(LogTemp, Warning, TEXT("Nova quest adaptativa gerada: '%s' (Dificuldade: %d)"), 
               *NewQuest.QuestName, (int32)NewQuest.Difficulty);
    }
}

void AQuest_AdaptiveQuestController::UpdatePlayerSkills(EQuest_QuestCategory Category, bool bSuccess, float CompletionTime)
{
    PlayerSkills.QuestsCompleted++;
    
    // Actualizar taxa de sucesso
    float OldSuccessRate = PlayerSkills.SuccessRate;
    PlayerSkills.SuccessRate = (OldSuccessRate * (PlayerSkills.QuestsCompleted - 1) + (bSuccess ? 1.0f : 0.0f)) / PlayerSkills.QuestsCompleted;
    
    // Actualizar tempo médio de conclusão
    float OldAvgTime = PlayerSkills.AverageCompletionTime;
    PlayerSkills.AverageCompletionTime = (OldAvgTime * (PlayerSkills.QuestsCompleted - 1) + CompletionTime) / PlayerSkills.QuestsCompleted;
    
    // Actualizar nível da categoria específica
    if (bSuccess)
    {
        switch (Category)
        {
            case EQuest_QuestCategory::Survival:
                PlayerSkills.SurvivalLevel = FMath::Min(10, PlayerSkills.SurvivalLevel + 1);
                break;
            case EQuest_QuestCategory::Hunting:
                PlayerSkills.HuntingLevel = FMath::Min(10, PlayerSkills.HuntingLevel + 1);
                break;
            case EQuest_QuestCategory::Exploration:
                PlayerSkills.ExplorationLevel = FMath::Min(10, PlayerSkills.ExplorationLevel + 1);
                break;
            case EQuest_QuestCategory::Crafting:
                PlayerSkills.CraftingLevel = FMath::Min(10, PlayerSkills.CraftingLevel + 1);
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Skills actualizadas - Taxa sucesso: %.2f, Tempo médio: %.1fs"), 
           PlayerSkills.SuccessRate, PlayerSkills.AverageCompletionTime);
}

EQuest_QuestDifficulty AQuest_AdaptiveQuestController::CalculateOptimalDifficulty(EQuest_QuestCategory Category)
{
    int32 CategoryLevel = 1;
    
    switch (Category)
    {
        case EQuest_QuestCategory::Survival:
            CategoryLevel = PlayerSkills.SurvivalLevel;
            break;
        case EQuest_QuestCategory::Hunting:
            CategoryLevel = PlayerSkills.HuntingLevel;
            break;
        case EQuest_QuestCategory::Exploration:
            CategoryLevel = PlayerSkills.ExplorationLevel;
            break;
        case EQuest_QuestCategory::Crafting:
            CategoryLevel = PlayerSkills.CraftingLevel;
            break;
    }
    
    // Ajustar dificuldade baseada na taxa de sucesso
    float SuccessRate = PlayerSkills.SuccessRate;
    
    if (SuccessRate > 0.8f && CategoryLevel >= 3)
    {
        return EQuest_QuestDifficulty::Advanced;
    }
    else if (SuccessRate > 0.6f && CategoryLevel >= 2)
    {
        return EQuest_QuestDifficulty::Intermediate;
    }
    else if (SuccessRate < 0.3f)
    {
        return EQuest_QuestDifficulty::Beginner;
    }
    
    return EQuest_QuestDifficulty::Intermediate;
}

FQuest_AdaptiveQuestData AQuest_AdaptiveQuestController::CreateSurvivalQuest(EQuest_QuestDifficulty Difficulty)
{
    FQuest_AdaptiveQuestData Quest;
    Quest.Category = EQuest_QuestCategory::Survival;
    Quest.Difficulty = Difficulty;
    
    TArray<FString> SurvivalTasks = {
        TEXT("Sobreviver durante 5 minutos sem perder vida"),
        TEXT("Encontrar água potável e beber"),
        TEXT("Construir um abrigo temporário"),
        TEXT("Fazer uma fogueira para aquecimento"),
        TEXT("Encontrar comida e consumir")
    };
    
    int32 TaskIndex = FMath::RandRange(0, SurvivalTasks.Num() - 1);
    Quest.QuestName = FString::Printf(TEXT("Sobrevivência: %s"), *SurvivalTasks[TaskIndex]);
    Quest.QuestDescription = FString::Printf(TEXT("Missão de sobrevivência: %s. Dificuldade: %s"), 
                                           *SurvivalTasks[TaskIndex], 
                                           *UEnum::GetValueAsString(Difficulty));
    
    // Ajustar parâmetros baseados na dificuldade
    switch (Difficulty)
    {
        case EQuest_QuestDifficulty::Beginner:
            Quest.TimeLimit = 600.0f; // 10 minutos
            Quest.RewardExperience = 50;
            break;
        case EQuest_QuestDifficulty::Intermediate:
            Quest.TimeLimit = 450.0f; // 7.5 minutos
            Quest.RewardExperience = 100;
            break;
        case EQuest_QuestDifficulty::Advanced:
            Quest.TimeLimit = 300.0f; // 5 minutos
            Quest.RewardExperience = 200;
            break;
        case EQuest_QuestDifficulty::Expert:
            Quest.TimeLimit = 180.0f; // 3 minutos
            Quest.RewardExperience = 350;
            break;
    }
    
    Quest.TargetLocation = GetSavanaLocation();
    return Quest;
}

FQuest_AdaptiveQuestData AQuest_AdaptiveQuestController::CreateHuntingQuest(EQuest_QuestDifficulty Difficulty)
{
    FQuest_AdaptiveQuestData Quest;
    Quest.Category = EQuest_QuestCategory::Hunting;
    Quest.Difficulty = Difficulty;
    
    TArray<FString> HuntingTargets = {
        TEXT("Caçar um pequeno herbívoro"),
        TEXT("Rastrear pegadas de dinossauro"),
        TEXT("Emboscar um predador menor"),
        TEXT("Colher recursos de uma carcaça"),
        TEXT("Afugentar predadores do território")
    };
    
    int32 TargetIndex = FMath::RandRange(0, HuntingTargets.Num() - 1);
    Quest.QuestName = FString::Printf(TEXT("Caça: %s"), *HuntingTargets[TargetIndex]);
    Quest.QuestDescription = FString::Printf(TEXT("Missão de caça: %s. Use estratégia e cautela."), 
                                           *HuntingTargets[TargetIndex]);
    
    switch (Difficulty)
    {
        case EQuest_QuestDifficulty::Beginner:
            Quest.TimeLimit = 480.0f;
            Quest.RewardExperience = 75;
            Quest.RequiredItems.Add(TEXT("Lança de madeira"));
            break;
        case EQuest_QuestDifficulty::Intermediate:
            Quest.TimeLimit = 360.0f;
            Quest.RewardExperience = 150;
            Quest.RequiredItems.Add(TEXT("Armadilha"));
            break;
        case EQuest_QuestDifficulty::Advanced:
            Quest.TimeLimit = 240.0f;
            Quest.RewardExperience = 250;
            Quest.RequiredItems.Add(TEXT("Arco primitivo"));
            break;
        case EQuest_QuestDifficulty::Expert:
            Quest.TimeLimit = 180.0f;
            Quest.RewardExperience = 400;
            Quest.RequiredItems.Add(TEXT("Veneno para setas"));
            break;
    }
    
    Quest.TargetLocation = GetForestLocation();
    return Quest;
}

FQuest_AdaptiveQuestData AQuest_AdaptiveQuestController::CreateExplorationQuest(EQuest_QuestDifficulty Difficulty)
{
    FQuest_AdaptiveQuestData Quest;
    Quest.Category = EQuest_QuestCategory::Exploration;
    Quest.Difficulty = Difficulty;
    
    TArray<FString> ExplorationGoals = {
        TEXT("Descobrir uma nova área do mapa"),
        TEXT("Encontrar uma fonte de água fresca"),
        TEXT("Localizar uma caverna segura"),
        TEXT("Mapear território de dinossauros"),
        TEXT("Encontrar materiais raros")
    };
    
    int32 GoalIndex = FMath::RandRange(0, ExplorationGoals.Num() - 1);
    Quest.QuestName = FString::Printf(TEXT("Exploração: %s"), *ExplorationGoals[GoalIndex]);
    Quest.QuestDescription = FString::Printf(TEXT("Missão de exploração: %s. Mantenha-se alerta aos perigos."), 
                                           *ExplorationGoals[GoalIndex]);
    
    // Escolher bioma baseado na dificuldade
    switch (Difficulty)
    {
        case EQuest_QuestDifficulty::Beginner:
            Quest.TimeLimit = 720.0f;
            Quest.RewardExperience = 60;
            Quest.TargetLocation = GetSavanaLocation();
            break;
        case EQuest_QuestDifficulty::Intermediate:
            Quest.TimeLimit = 540.0f;
            Quest.RewardExperience = 120;
            Quest.TargetLocation = GetForestLocation();
            break;
        case EQuest_QuestDifficulty::Advanced:
            Quest.TimeLimit = 360.0f;
            Quest.RewardExperience = 220;
            Quest.TargetLocation = GetSwampLocation();
            break;
        case EQuest_QuestDifficulty::Expert:
            Quest.TimeLimit = 300.0f;
            Quest.RewardExperience = 380;
            Quest.TargetLocation = GetMountainLocation();
            break;
    }
    
    return Quest;
}

FQuest_AdaptiveQuestData AQuest_AdaptiveQuestController::CreateCraftingQuest(EQuest_QuestDifficulty Difficulty)
{
    FQuest_AdaptiveQuestData Quest;
    Quest.Category = EQuest_QuestCategory::Crafting;
    Quest.Difficulty = Difficulty;
    
    TArray<FString> CraftingTasks = {
        TEXT("Criar uma ferramenta de pedra"),
        TEXT("Construir um abrigo básico"),
        TEXT("Fazer uma armadilha para pequenos animais"),
        TEXT("Preparar medicamento natural"),
        TEXT("Criar arma de longo alcance")
    };
    
    int32 TaskIndex = FMath::RandRange(0, CraftingTasks.Num() - 1);
    Quest.QuestName = FString::Printf(TEXT("Crafting: %s"), *CraftingTasks[TaskIndex]);
    Quest.QuestDescription = FString::Printf(TEXT("Missão de crafting: %s. Reúna os materiais necessários."), 
                                           *CraftingTasks[TaskIndex]);
    
    switch (Difficulty)
    {
        case EQuest_QuestDifficulty::Beginner:
            Quest.TimeLimit = 420.0f;
            Quest.RewardExperience = 80;
            Quest.RequiredItems = {TEXT("Pedra"), TEXT("Graveto")};
            break;
        case EQuest_QuestDifficulty::Intermediate:
            Quest.TimeLimit = 300.0f;
            Quest.RewardExperience = 160;
            Quest.RequiredItems = {TEXT("Pedra afiada"), TEXT("Corda vegetal"), TEXT("Madeira")};
            break;
        case EQuest_QuestDifficulty::Advanced:
            Quest.TimeLimit = 240.0f;
            Quest.RewardExperience = 280;
            Quest.RequiredItems = {TEXT("Sílex"), TEXT("Osso"), TEXT("Tendão"), TEXT("Resina")};
            break;
        case EQuest_QuestDifficulty::Expert:
            Quest.TimeLimit = 180.0f;
            Quest.RewardExperience = 450;
            Quest.RequiredItems = {TEXT("Metal primitivo"), TEXT("Couro curtido"), TEXT("Veneno natural")};
            break;
    }
    
    Quest.TargetLocation = GetSavanaLocation();
    return Quest;
}

bool AQuest_AdaptiveQuestController::StartQuest(const FQuest_AdaptiveQuestData& Quest)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        return false;
    }
    
    FQuest_AdaptiveQuestData NewActiveQuest = Quest;
    NewActiveQuest.bIsActive = true;
    
    ActiveQuests.Add(NewActiveQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest iniciada: '%s' - Tempo limite: %.1f segundos"), 
           *NewActiveQuest.QuestName, NewActiveQuest.TimeLimit);
    
    return true;
}

bool AQuest_AdaptiveQuestController::CompleteQuest(const FString& QuestName, bool bSuccess)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestName == QuestName && ActiveQuests[i].bIsActive)
        {
            FQuest_AdaptiveQuestData CompletedQuest = ActiveQuests[i];
            float CompletionTime = CompletedQuest.TimeLimit; // Tempo restante
            
            UpdatePlayerSkills(CompletedQuest.Category, bSuccess, CompletionTime);
            
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest '%s' completada - Sucesso: %s, XP: %d"), 
                   *QuestName, bSuccess ? TEXT("Sim") : TEXT("Não"), 
                   bSuccess ? CompletedQuest.RewardExperience : 0);
            
            return true;
        }
    }
    
    return false;
}

void AQuest_AdaptiveQuestController::CancelQuest(const FString& QuestName)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestName == QuestName)
        {
            ActiveQuests.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Quest '%s' cancelada"), *QuestName);
            break;
        }
    }
}

TArray<FQuest_AdaptiveQuestData> AQuest_AdaptiveQuestController::GetActiveQuests() const
{
    return ActiveQuests;
}

FVector AQuest_AdaptiveQuestController::GetRandomLocationInBiome(const FString& BiomeName)
{
    if (BiomeName == TEXT("Savana"))
    {
        return GetSavanaLocation();
    }
    else if (BiomeName == TEXT("Floresta"))
    {
        return GetForestLocation();
    }
    else if (BiomeName == TEXT("Pantano"))
    {
        return GetSwampLocation();
    }
    else if (BiomeName == TEXT("Deserto"))
    {
        return GetDesertLocation();
    }
    else if (BiomeName == TEXT("Montanha"))
    {
        return GetMountainLocation();
    }
    
    return GetSavanaLocation(); // Default
}

void AQuest_AdaptiveQuestController::AnalyzePlayerPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("=== ANÁLISE DE PERFORMANCE DO JOGADOR ==="));
    UE_LOG(LogTemp, Log, TEXT("Quests completadas: %d"), PlayerSkills.QuestsCompleted);
    UE_LOG(LogTemp, Log, TEXT("Taxa de sucesso: %.2f%%"), PlayerSkills.SuccessRate * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Tempo médio: %.1f segundos"), PlayerSkills.AverageCompletionTime);
    UE_LOG(LogTemp, Log, TEXT("Níveis - Sobrevivência: %d, Caça: %d, Exploração: %d, Crafting: %d"), 
           PlayerSkills.SurvivalLevel, PlayerSkills.HuntingLevel, 
           PlayerSkills.ExplorationLevel, PlayerSkills.CraftingLevel);
}

void AQuest_AdaptiveQuestController::InitializeQuestTemplates()
{
    UE_LOG(LogTemp, Log, TEXT("Quest templates inicializados"));
}

FVector AQuest_AdaptiveQuestController::GetSavanaLocation()
{
    FVector BaseLocation(-0.0f, 0.0f, 100.0f);
    FVector RandomOffset(
        FMath::RandRange(-15000.0f, 15000.0f),
        FMath::RandRange(-15000.0f, 15000.0f),
        0.0f
    );
    return BaseLocation + RandomOffset;
}

FVector AQuest_AdaptiveQuestController::GetForestLocation()
{
    FVector BaseLocation(-45000.0f, 40000.0f, 100.0f);
    FVector RandomOffset(
        FMath::RandRange(-10000.0f, 10000.0f),
        FMath::RandRange(-10000.0f, 10000.0f),
        0.0f
    );
    return BaseLocation + RandomOffset;
}

FVector AQuest_AdaptiveQuestController::GetSwampLocation()
{
    FVector BaseLocation(-50000.0f, -45000.0f, 50.0f);
    FVector RandomOffset(
        FMath::RandRange(-8000.0f, 8000.0f),
        FMath::RandRange(-8000.0f, 8000.0f),
        0.0f
    );
    return BaseLocation + RandomOffset;
}

FVector AQuest_AdaptiveQuestController::GetDesertLocation()
{
    FVector BaseLocation(55000.0f, 0.0f, 150.0f);
    FVector RandomOffset(
        FMath::RandRange(-12000.0f, 12000.0f),
        FMath::RandRange(-12000.0f, 12000.0f),
        0.0f
    );
    return BaseLocation + RandomOffset;
}

FVector AQuest_AdaptiveQuestController::GetMountainLocation()
{
    FVector BaseLocation(40000.0f, 50000.0f, 800.0f);
    FVector RandomOffset(
        FMath::RandRange(-8000.0f, 8000.0f),
        FMath::RandRange(-8000.0f, 8000.0f),
        FMath::RandRange(-200.0f, 400.0f)
    );
    return BaseLocation + RandomOffset;
}